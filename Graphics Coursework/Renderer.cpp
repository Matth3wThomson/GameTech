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
	if (!InitShadowBuffers())
		return;
	if (!InitSceneObjects())
		return;
	if (!InitSkybox())
		return;
	if (!InitWater())
		return;
	/*if (!InitShadowBuffers())
		return;*/
	if (!InitPostProcess())
		return;
	if (!InitDebug())
		return;

	//BLOOM TESTING!
	/*glGenTextures(1, &bloomTex);
	glBindTexture(GL_TEXTURE_2D, bloomTex);*/

	//PARTICLE STUFF

	if (!InitParticles()) 
		return;
	/*particleShader = new Shader(SHADERDIR"basicVertex.glsl", 
	SHADERDIR"colourFragment.glsl", SHADERDIR"particleEmitGeom.glsl");

	if (!particleShader->LinkProgram()) return;

	emitter = new ParticleEmitter();

	emitter->SetParticleSize(8.0f);
	emitter->SetParticleVariance(1.0f);
	emitter->SetLaunchParticles(16.0f);
	emitter->SetParticleLifetime(2000.0f);
	emitter->SetParticleSpeed(0.1f);*/

	//Turn on depth testing
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Initialize a projection matrix 
	cameraProjMat = projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float) width / (float) height, 45.0f);
	ortho = Matrix4::Orthographic(-1.0f,1.0f,(float)width, 0.0f,(float)height, 0.0f);

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
	/*emitter->Update(msec);*/
	UpdateParticles(msec);

	if (!pause){
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_2)){
			movementVar += msec*0.0001f;

		} else {
			movementVar += msec*0.001f;
		}


		UpdateSceneObjects(msec);
	}

	//TEXT
	UpdatePostProcess(msec);
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
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, GetDrawTarget(), 0);

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

	//Bind our depth texture from our shadow FBO to texture unit 2
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	viewMatrix = camera->BuildViewMatrix();
	projMatrix = cameraProjMat;
	textureMatrix.ToIdentity();

	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	BuildNodeLists(root, camera->GetPosition());
	SortNodeLists();

	//Draw the opaque nodes with the specific shader uploads
	for (auto itr = nodeList.begin(); itr != nodeList.end(); ++itr){
		SetCurrentShader(sceneShader);

		Matrix4 tempMatrix = shadowVPMatrix * (*itr)->GetWorldTransform();
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
			"shadowVPMatrix"),1,false, tempMatrix.values);
		(*itr)->Draw(*this);

		if (debug){ 
			if (drawBound) DrawBounds(*itr);
			if ((*itr)->GetMesh()) objectsDrawn++;
		}
	}

	for (auto itr = transparentNodes.rbegin(); itr != transparentNodes.rend(); ++itr){
		SetCurrentShader(sceneShader);

		Matrix4 tempMatrix = shadowVPMatrix * (*itr)->GetWorldTransform();
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
			"shadowVPMatrix"),1,false, tempMatrix.values);
		(*itr)->Draw(*this);

		if (debug){
			if (drawBound) DrawBounds(*itr);
			if ((*itr)->GetMesh()) objectsDrawn++;
		}
	}

	/*if (debug)
		for (auto itr = nodeList.begin(); itr != nodeList.end(); ++itr){
			if ((*itr)->GetMesh()) objectsDrawn++;
			if (drawBound) DrawBounds(*itr);
		}*/

	ClearNodeLists();

	DrawWater(false);
	//DrawParticleEmitter(emitter);

	PPDrawn();
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
		projMatrix = ortho;
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
	projMatrix = cameraProjMat;
	UpdateShaderMatrices();

	sphere->Draw();
	objectsDrawn++;

	glUseProgram(0);
}

