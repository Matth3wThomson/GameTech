#include "Renderer.h"

bool Renderer::debug = true;
Renderer* Renderer::instance = NULL;

/**
NOTES:
-This cpp file contains the methods that are too generic to be specialized.
*/

Renderer::Renderer(Window& parent) : OGLRenderer(parent)
{
	//Generic Renderer + functionaility properties
	timeOfDay = 0.0f;
	anim = 0;
	pause = false;
	timeSlowed = false;
	dayTimeSpeedIncrease = false;
	rotation = 0.0f;

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

	if (!InitSceneObjects())
		return;

	if (!InitSkybox())
		return;

	if (!InitWater())
		return;

	if (!InitPostProcess())
		return;

	if (!InitDebug())
		return;

	if (!InitParticles()) 
		return;

	if (!InitDeferredRendering()){
		return;
	}

	//Initialize a projection matrix 
	cameraProjMat = projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float) width / (float) height, 45.0f);
	ortho = Matrix4::Orthographic(-1.0f,1.0f,(float)width, 0.0f,(float)height, 0.0f);

	//Set our generic openGL values
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	init = true;
}


Renderer::~Renderer(void)
{

	DeleteDebug();
	DeletePostProcess();
	DeleteParticles();
	DeleteWater();
	DeleteSkybox();
	DeleteDeferredRendering();
	DeleteSceneObjects();
	DeleteShadowBuffers();

	delete light;
	delete phong;
	delete passThrough;

	currentShader = NULL;
}

void Renderer::UpdateScene(float msec){

	camera->UpdateCamera(msec);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P))
		pause = !pause;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NUMPAD9))
		debug = !debug;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))
		timeSlowed = !timeSlowed;


	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3))
		dayTimeSpeedIncrease = !dayTimeSpeedIncrease;


	if (timeSlowed)	msec *= 0.1f;

	if (!pause){

		if (dayTimeSpeedIncrease) timeOfDay += msec * 0.001f;
		else timeOfDay += msec*0.00001f;

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NUMPAD0)){
			timeOfDay = 0;
		}

		//One day is 2 PI (I bet you can't guess why ;) )
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
	UpdateDebug();

}

void Renderer::RenderScene(){


	DrawShadowScene(); //Create our shadow depth tex from the first render pass

	DrawCombinedScene(); //Use this value and compute forward lighting

	DrawPointLights();	//Apply our pointlights (deferred rendering) lights to
	//Emissive and specular textures

	CombineBuffers();	//Combine all rendering information so far to compute final image

	DrawPostProcess(); //Draw our post processing effects

	PresentScene(); //Render the final image to the screen

	//If debugging, overlay the debugging information
	if (debug){
		DrawDebugOverlay();
	}

	projMatrix = cameraProjMat;

	SwapBuffers(); //Finally swap the buffers and prepare for the next frame!
}

void Renderer::DrawCombinedScene(){

	//Bind the FBO we will draw to
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, GetDrawTarget(), 0);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//Draw our skybox first
	DrawSkybox();

	//Create our view and proj matrices and cull any objects outside our viewing frustum
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = cameraProjMat;
	textureMatrix.ToIdentity();

	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	BuildNodeLists(root, camera->GetPosition());
	SortNodeLists();

	//Draw the opaque nodes first front to back
	//glDisable(GL_CULL_FACE); //TODO: Turn this on?

	if (lineMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (drawWorld){
		for (auto itr = nodeList.begin(); itr != nodeList.end(); ++itr){
			//SetCurrentShader(phong);
			SetCurrentShader(sceneShader);
			//SetCurrentShader(passThrough);

			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			(*itr)->Draw(*this, true);

			if (debug){ 
				if (drawBound) DrawBounds(*itr);
				if ((*itr)->GetMesh()) objectsDrawn++;
			}
		}

		//Draw the transparent nodes second, back to front
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);	//TODO: Is this even necessary?
		for (auto itr = transparentNodes.rbegin(); itr != transparentNodes.rend(); ++itr){
			SetCurrentShader(sceneShader);

			(*itr)->Draw(*this);

			if (debug){
				if (drawBound) DrawBounds(*itr);
				if ((*itr)->GetMesh()) objectsDrawn++;
			}
		}
		glDepthMask(GL_TRUE);
	}

	if (physicsDrawing){
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		SetCurrentShader(phong);
		DrawPhysics();
	}

	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	ClearNodeLists();

	Renderer::GetRenderer().DrawDebugCross(DEBUGDRAW_PERSPECTIVE,
		Vector3(0,0,0), Vector3(300, 300, 300), Vector3(1,1,1));

	DrawDebugPerspective();

	//Signify to the PP system that the buffers have been "pinged"
	PPDrawn();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

//Updates all relevant shaders per frame with uploads that
//arent specific to an object

//TODO: Have an array or vector of per frame shader updates to call?
void Renderer::UpdateShadersPerFrame(){
	UpdateGenericShadersPF();
	UpdateCombineSceneShaderMatricesPF();
	UpdateHeightMapShaderPF();
	UpdateWaterShaderMatricesPF();
}

//Draws a supplied string to the screen
void Renderer::DrawString(const std::string& text, const Vector3& pos, const float size, const bool perspective){

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

//Update method for the generic shader matrices per frame.
void Renderer::UpdateGenericShadersPF(){
	SetCurrentShader(phong);

	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float*) &camera->GetPosition());

	SetShaderLight(*light);
}