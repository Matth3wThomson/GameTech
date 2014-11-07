#include "Renderer.h"


Renderer::Renderer(Window& parent) : OGLRenderer(parent)
{
	timePassed = 0;
	anim = 0;

	camera = new Camera(-8.0f, -25.0f, Vector3(-200.0f, 50.0f, 250.0f));

	light = new Light(Vector3(-450.0f, 200.0f, 280.0f),
		Vector4(1,1,1,1), 5500.0f);

	/*light->SetPosition(Vector3(-200.0f, 700.0f, 250.0f));*/
	

	hellData = new MD5FileData(MESHDIR"hellknight.md5mesh");
	hellNode = new MD5Node(*hellData);

	hellData->AddAnim(MESHDIR"idle2.md5anim");
	hellData->AddAnim(MESHDIR"attack2.md5anim");
	hellData->AddAnim(MESHDIR"pain1.md5anim");
	hellData->AddAnim(MESHDIR"roar1.md5anim");
	hellData->AddAnim(MESHDIR"stand.md5anim");
	hellData->AddAnim(MESHDIR"walk7.md5anim");

	//hellNode->PlayAnim(MESHDIR"idle2.md5anim");

	sceneShader = new Shader(SHADERDIR"shadowSceneVert.glsl",
		SHADERDIR"shadowSceneFrag.glsl");

	shadowShader = new Shader(SHADERDIR"shadowVert.glsl", SHADERDIR"shadowFrag.glsl");

	if (!sceneShader->LinkProgram() || !shadowShader->LinkProgram())
		return;

	//Generate and bind a texture for our shadow FBO
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	//Remove all filtering
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Set the textures size and bits to use
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	//Tell gl the compare mode; gl compare R to texture (depth value of the texture)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
		GL_COMPARE_R_TO_TEXTURE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//Generate and bind a frame buffer
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	//Bind the shadow texture to the frame buffer in the depth part
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, shadowTex, 0);

	//Tell gl we dont want colours drawn to this frame buffer
	glDrawBuffer(GL_NONE);

	//Unbind the frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Generate a floor quad, passing a texture and bump map
	floor = Mesh::GenerateQuad(); //TODO: not index buffered?
	floor->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"brick.tga",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	floor->SetBumpMap(SOIL_load_OGL_texture(TEXTUREDIR"brickDOT3.tga",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	
	root = new SceneNode();

	/*SceneNode**/ floorNode = new SceneNode(floor);
	floorNode->SetTransform(Matrix4::Rotation(90, Vector3(1,0,0)) *
		Matrix4::Scale(Vector3(450, 450, 1)));
	floorNode->SetBoundingRadius(620.0f);

	hellNode->SetTransform(Matrix4::GetIdentitiy());
	hellNode->SetBoundingRadius(100);

	root->AddChild(floorNode);
	root->AddChild(hellNode);

	//Turn on depth testing
	glEnable(GL_DEPTH_TEST);

	//Initialize a projection matrix 
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float) width / (float) height, 45.0f);

	init = true;
}


Renderer::~Renderer(void)
{
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);

	delete camera;
	delete light;
	delete hellData;
	delete hellNode;
	delete floor;

	delete sceneShader;
	delete shadowShader;
	currentShader = NULL;
}

void Renderer::UpdateScene(float msec){
	timePassed += msec*0.001;
	camera->UpdateCamera(msec);
	hellNode->Update(msec);
	light->SetPosition(Vector3(-450.0f * sin(timePassed), 200.0f + 200.0f * sin(timePassed*0.1), 280.0f * cos(timePassed)));

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)){
		++anim %= 6;
		switch(anim){
			case 0: hellNode->PlayAnim(MESHDIR"idle2.md5anim"); break;
			case 1: hellNode->PlayAnim(MESHDIR"roar1.md5anim"); break;
			case 2: hellNode->PlayAnim(MESHDIR"attack2.md5anim"); break;
			case 3: hellNode->PlayAnim(MESHDIR"pain1.md5anim"); break;
			case 4: hellNode->PlayAnim(MESHDIR"walk7.md5anim"); break;
			case 5: hellNode->PlayAnim(MESHDIR"stand.md5anim"); break;
		}
	}
	//light->SetPosition(camera->GetPosition());

}

void Renderer::RenderScene(){
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//Build node lists in order of distance from the light
	DrawShadowScene(); //First Render pass

	//Build node lists in order of distance from the camera
	DrawCombinedScene(); //Second render pass


	SwapBuffers();
}

void Renderer::DrawShadowScene(){

	//Draw to our shadow framebuffer (with shadow texture)
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	//Clear the old shadow texture
	glClear(GL_DEPTH_BUFFER_BIT);

	//Set our viewport to be the size of the shadow texture
	glViewport(0,0,SHADOWSIZE,SHADOWSIZE);

	//Turn off colour writing!
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	SetCurrentShader(shadowShader);

	//Create a view matrix from the light position facing the origin
	viewMatrix = Matrix4::BuildViewMatrix(
		light->GetPosition(), Vector3(0,0,0));

	//Stores the shadows vp matrix multiplied by the bias matrix
	//to keep coordinates in clip space range
	textureMatrix = biasMatrix*(projMatrix*viewMatrix);

	//Update our shaders matrices
	//UpdateShaderMatrices();

	//Draw the floor and the hellknight
	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	BuildNodeLists(root, light->GetPosition());
	SortNodeLists();
	DrawNodes();
	ClearNodeLists();

	//Disable our shader, turn colour writes back on and set our view port back to
	//our window size
	glUseProgram(0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0,0,width, height);

	//Unbind our shadow frame buffer too
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawCombinedScene(){
	
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
	//UpdateShaderMatrices();

	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	BuildNodeLists(root, light->GetPosition());
	SortNodeLists();
	DrawNodes();
	ClearNodeLists();

	glUseProgram(0);
}


void Renderer::DrawMesh(){
	modelMatrix.ToIdentity();

	Matrix4 tempMatrix = textureMatrix * modelMatrix;

	//Use texture Matrix as the model * shadow view projection matrix
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"textureMatrix"), 1, false, tempMatrix.values);

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"modelMatrix"), 1, false, modelMatrix.values);

	hellNode->Draw(*this);
}

void Renderer::DrawFloor(){

	modelMatrix = Matrix4::Rotation(90, Vector3(1,0,0)) *
		Matrix4::Scale(Vector3(450, 450, 1));

	//Use the texture matrix as the model * shadow view projection matrix
	Matrix4 tempMatrix = textureMatrix * modelMatrix;

	/*tempMatrix = Matrix4::Scale(Vector3(10,10,10));*/

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"textureMatrix"),1,false, tempMatrix.values);

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"modelMatrix"), 1, false, modelMatrix.values);

	floor->Draw();
}

void Renderer::BuildNodeLists(SceneNode* from, const Vector3& viewPos){
	if (frameFrustum.InsideFrustum(*from)){
		Vector3 dir = from->GetWorldTransform().GetPositionVector() -
			viewPos;

		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f)
			transparentNodes.push_back(from);
		else nodeList.push_back(from);

	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart();
		i != from->GetChildIteratorEnd(); ++i){
			BuildNodeLists((*i), viewPos);
	}
}

void Renderer::SortNodeLists(){
	std::sort(transparentNodes.begin(),
		transparentNodes.end(),
		SceneNode::CompareByCameraDistance);

	std::sort(nodeList.begin(),
		nodeList.end(),
		SceneNode::CompareByCameraDistance);
}

void Renderer::DrawNodes(){

	//Draw the nodes from closest to furthest away
	for (vector<SceneNode*>::const_iterator i = nodeList.begin();
		i!= nodeList.end(); ++i){
			DrawNode((*i));
	}

	//Draw the transparent nodes from furthest away to closest
	for (vector<SceneNode*>::const_reverse_iterator i = transparentNodes.rbegin();
		i != transparentNodes.rend(); ++i){
			DrawNode((*i));
	}
}

void Renderer::ClearNodeLists(){
	transparentNodes.clear();
	nodeList.clear();
}

void Renderer::DrawNode(SceneNode* n){
	if (n->GetMesh()){
		/*glUniformMatrix4fv(
			glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"),
			1, false, (float*) &(n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale())));*/
		/*glUniformMatrix4fv(
			glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"),
			1, false, (float*) &(n->GetWorldTransform()));*/

		modelMatrix = n->GetTransform();

		glUniform4fv ( glGetUniformLocation ( currentShader->GetProgram(),
			"nodeColour") ,1 ,( float *)& n->GetColour ());

		glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
			"useTexture"), (int)n->GetMesh()->GetTexture());

		UpdateShaderMatrices();

		Matrix4 tempMatrix = textureMatrix * modelMatrix;

		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"textureMatrix"),1,false, tempMatrix.values);

		

		n->Draw(*this);
	}
}