#include "Renderer.h"

bool Renderer::debug = true;

Renderer::Renderer(Window& parent) : OGLRenderer(parent)
{
	//Generic Renderer + functionaility properties
	//wglSwapIntervalEXT(0);
	movementVar = 0;
	anim = 0;
	pause = false;
	rotation = 0.0f;

	camera = new Camera(-8.0f, -25.0f, Vector3(-200.0f, 50.0f, 250.0f));

	light = new Light(Vector3(0.0f, 5000.0f, 0.0f),
		Vector4(1,1,0.7f,1), 55000.0f);

	passThrough = new Shader(SHADERDIR"TexturedVertex.glsl", SHADERDIR"TexturedFragment.glsl");

	if (!passThrough->LinkProgram())
		return;

	//Initialise parts of the scene!
	if (!InitSceneObjects())
		return;
	if (!InitSkybox())
		return;
	if (!InitWater())
		return;
	if (!InitShadowBuffers())
		return;
	if (!InitPostProcess())
		return;
	if (!InitDebug())
		return;

	//TEMPORARY	
	/*quadPos = Vector3(-1115.9f,141,-1501.6f);
	scale = Vector3(939.3f,554.9f,1);
	rotation = 90;*/

	//Turn on depth testing
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Initialize a projection matrix 
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float) width / (float) height, 45.0f);

	init = true;
}


Renderer::~Renderer(void)
{
	DeleteDebug();
	DeletePostProcess();
	DeleteShadowBuffers();
	DeleteWater();
	DeleteSkybox();
	DeleteSceneObjects();
}

void Renderer::UpdateScene(float msec){

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P))
		pause = !pause;

	camera->UpdateCamera(msec);

	
	if (!pause){
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_2)){
			movementVar += msec*0.0001f;

		} else {
			movementVar += msec*0.001f;
		}

		//TEMPORARY
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_UP))
			quadPos.z += msec*0.1f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_DOWN))
			quadPos.z -= msec*0.1f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT))
			quadPos.x += msec*0.1f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))
			quadPos.x -= msec*0.1f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_PLUS))
			quadPos.y += msec*0.1f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_MINUS))
			quadPos.y -= msec*0.1f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_INSERT))
			scale.x += msec*0.1f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_HOME))
			scale.y += msec*0.1f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_DELETE))
			scale.x -= msec*0.1f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_END))
			scale.y -= msec*0.1f;
		

		UpdateSceneObjects(msec);
	}

	//TEXT
	UpdateDebug(msec);

}

void Renderer::RenderScene(){

	//Build node lists in order of distance from the light
	DrawShadowScene(); //First Render pass

	//Build node lists in order of distance from the camera
	DrawCombinedScene(); //Second render pass

	//Draw our post processing effects
	DrawPostProcess();

	//Render the final image to the screen
	PresentScene();

	//If debugging, overlay the debugging information
	if (debug){
		DrawDebugOverlay();
	}

	SwapBuffers();
}

void Renderer::DrawCombinedScene(){

	//Bind the FBO we will draw to
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//TODO: Sort out drawing the frustum!
	////Draw the light frustum
	//DrawFrustum();

	DrawSkybox();

	SetCurrentShader(sceneShader);

	//Upload our textures to units 0-2
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"bumpTex"), 1);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"shadowTex"), 2);

	//upload our cameras position
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());

	SetShaderLight(*light);

	//SetCurrentShader(shadowShader);
	//SetCurrentShader(sceneShader);

	//Bind our depth texture from our shadow FBO to texture unit 2
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float) width / (float) height, 45.0f);
	textureMatrix.ToIdentity();

	/*shadowVPMatrix = biasMatrix*(projMatrix*viewMatrix);*/

	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	BuildNodeLists(root, camera->GetPosition());
	SortNodeLists();
	DrawNodes();

	if (debug)
		for (auto itr = nodeList.begin(); itr != nodeList.end(); ++itr)
			if ((*itr)->GetMesh()) objectsDrawn++;

	ClearNodeLists();
	
	DrawWater(false);
	

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

void Renderer::DrawString(const std::string& text, const Vector3& pos, const float size, const bool perspective){

	//TODO: Creation of a text mesh every time!?
	TextMesh* mesh = new TextMesh(text, *basicFont);

	if (perspective){
		modelMatrix = Matrix4::Translation(pos) * Matrix4::Scale(Vector3(size,size,1));

	} else {
		modelMatrix = Matrix4::Translation(Vector3(pos.x, height-pos.y, pos.z))
			*Matrix4::Scale(Vector3(size,size,1));
		viewMatrix.ToIdentity();
		projMatrix = Matrix4::Orthographic(-1.0f,1.0f,(float)width, 0.0f,(float)height, 0.0f);
	}

	////Either way, we update the matrices, and draw the mesh
	UpdateShaderMatrices();
	mesh->Draw();

	delete mesh; //Once it's drawn, we don't need it anymore!

}

//TODO: Drawing light source no matter what!?
void Renderer::DrawLight(const Light* light){
	SetCurrentShader(passThrough);

	modelMatrix = Matrix4::Translation(light->GetPosition()) *
		Matrix4::Scale(Vector3(100.0f, 100.0f, 100.0f));
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float) width / (float) height, 45.0f);
	UpdateShaderMatrices();

	sphere->Draw();
	objectsDrawn++;

	glUseProgram(0);
}


