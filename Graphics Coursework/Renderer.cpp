#include "Renderer.h"

bool Renderer::debug = true;

Renderer::Renderer(Window& parent) : OGLRenderer(parent)
{
	//Generic Renderer + functionaility properties
	//wglSwapIntervalEXT(0);
	timeOfDay = 0.0f;
	anim = 0;
	pause = false;
	toon = false;
	timeSlowed = false;
	lightTimeSlowed = false;
	rotation = 0.0f;

	std::cout << "START gl error: " << glGetError() << std::endl;

	camera = new Camera(-8.0f, -25.0f, Vector3(0.0f, 500.0f, -250.0f));

	light = new Light(Vector3(0.0f, 5000.0f, 0.0f),
		Vector4(1,1,0.7f,1), 55000.0f);

	phong = new Shader(SHADERDIR"PerPixelVertex.glsl", SHADERDIR"PerPixelFragment.glsl");

	if (!phong->LinkProgram())
		return;

	SetCurrentShader(phong);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	passThrough = new Shader(SHADERDIR"TexturedVertex.glsl", SHADERDIR"TexturedFragment.glsl");

	if (!passThrough->LinkProgram())
		return;

	SetCurrentShader(passThrough);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	//Initialise parts of the scene!
	if (!InitShadowBuffers())
		return;

	if (!InitSceneObjects()) //ERROR
		return;

	if (!InitSkybox())	//ERROR
		return;

	if (!InitWater()) //ERROR
		return;

	if (!InitPostProcess())
		return;

	if (!InitDebug()) //ERROR
		return;

	if (!InitParticles()) 
		return;

	//Turn on depth testing
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Initialize a projection matrix 
	cameraProjMat = projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float) width / (float) height, 45.0f);
	ortho = Matrix4::Orthographic(-1.0f,1.0f,(float)width, 0.0f,(float)height, 0.0f);

	tree1 = new TreeNode(particleShader, phong);
	tree1->SetShader(sceneShader);
	tree1->SetUpdateShaderFunction([this]{ UpdateCombineSceneShaderMatricesPO(); } );
	tree1->SetPosition(Vector3(700, 35, 800));

	root->AddChild(tree1);

	tree2 = new TreeNode(particleShader, phong);
	tree2->SetShader(sceneShader);
	tree2->SetUpdateShaderFunction([this]{ UpdateCombineSceneShaderMatricesPO(); } );
	tree2->SetPosition(Vector3(0, 35, 1500));

	root->AddChild(tree2);

	//ADDITION OF DEFERRED RENDERING!
	if (!InitDeferredRendering()){
		return;
	}

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

	float debugMsec = msec;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P))
		pause = !pause;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_T)){
		toon = !toon;
		SwitchToToon(toon);
	}

	camera->UpdateCamera(msec);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))
		timeSlowed = !timeSlowed;
	

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3))
		lightTimeSlowed = !lightTimeSlowed;
	

	if (timeSlowed)	msec *= 0.1f;

	if (!pause){

		if (lightTimeSlowed) timeOfDay += msec * 0.0001f;
		else timeOfDay += msec*0.001f;

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NUMPAD0)){
			timeOfDay = 0;
			tree1->ResetTree();
			tree2->ResetTree();
		}


		timeOfDay = std::fmod(timeOfDay, 2 * PI);

		UpdateParticles(msec);
		UpdateSceneObjects(msec);
		UpdateWater(msec);
		UpdateSkybox(msec);
		UpdateDeferredRendering(msec);
	}

	UpdateShadersPerFrame();

	//TEXT
	UpdatePostProcess(msec);
	UpdateDebug(debugMsec);

}

void Renderer::RenderScene(){

	//Build node lists in order of distance from the light
	DrawShadowScene(); //First Render pass

	//Build node lists in order of distance from the camera
	DrawCombinedScene(); //Second render pass

	DrawPointLights();

	CombineBuffers();

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

	viewMatrix = camera->BuildViewMatrix();
	projMatrix = cameraProjMat;
	textureMatrix.ToIdentity();

	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	BuildNodeLists(root, camera->GetPosition());
	SortNodeLists();

	//Draw the opaque nodes with the specific shader uploads
	for (auto itr = nodeList.begin(); itr != nodeList.end(); ++itr){
		SetCurrentShader(sceneShader);
		(*itr)->Draw(*this);

		if (debug){ 
			if (drawBound) DrawBounds(*itr);
			if ((*itr)->GetMesh()) objectsDrawn++;
		}
	}

	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	for (auto itr = transparentNodes.rbegin(); itr != transparentNodes.rend(); ++itr){
		SetCurrentShader(sceneShader);
		(*itr)->Draw(*this);

		if (debug){
			if (drawBound) DrawBounds(*itr);
			if ((*itr)->GetMesh()) objectsDrawn++;
		}
	}
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);

	ClearNodeLists();

	PPDrawn();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

void Renderer::UpdateShadersPerFrame(){
	UpdateGenericShadersPF();
	UpdateWaterShaderMatricesPF();
	UpdateCombineSceneShaderMatricesPF();
	UpdateHeightMapShaderPF(); //NEW
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

void Renderer::UpdateGenericShadersPF(){
	SetCurrentShader(phong);

	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float*) &camera->GetPosition());

	SetShaderLight(*light);

}

void Renderer::SwitchToToon(bool toon){

	if (toon){
		heightMap->SetTexture(HMToonTex);
		heightMap->SetHighgroundTex(HMToonHighTex);
		tree1->SwitchToToon(toon);
		tree2->SwitchToToon(toon);
	} else {
		heightMap->SetTexture(heightMapTex);
		heightMap->SetHighgroundTex(heightMapHighTex);
		tree1->SwitchToToon(toon);
		tree2->SwitchToToon(toon);
	}
}

