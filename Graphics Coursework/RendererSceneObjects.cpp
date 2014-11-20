#include "Renderer.h"

bool Renderer::InitSceneObjects(){

	heightMapShader = new Shader(SHADERDIR"shadowSceneVert.glsl",
		SHADERDIR"heightMapShadowSceneFrag.glsl");

	if (!heightMapShader->LinkProgram())
		return false;

	SetCurrentShader(heightMapShader);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"bumpTex"), 1);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"shadowTex"), 2);

	hellData = new MD5FileData(MESHDIR"hellknight.md5mesh");
	hellNode = new MD5Node(*hellData);

	std::cout << "gl error: " << glGetError() << std::endl;

	hellData->AddAnim(MESHDIR"idle2.md5anim");
	hellData->AddAnim(MESHDIR"attack2.md5anim");
	hellData->AddAnim(MESHDIR"pain1.md5anim");
	hellData->AddAnim(MESHDIR"roar1.md5anim");
	hellData->AddAnim(MESHDIR"stand.md5anim");
	hellData->AddAnim(MESHDIR"walk7.md5anim");

	heightMap = new HeightMap(TEXTUREDIR"terrain.raw");

	quad = Mesh::GenerateQuad(); //TODO: not index buffered?
	sphere = new OBJMesh(MESHDIR"sphere.obj");

	std::cout << "gl error: " << glGetError() << std::endl;

	heightMapHighTex = SOIL_load_OGL_texture(
		TEXTUREDIR"Barren reds.jpg", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	 heightMapTex = SOIL_load_OGL_texture(
		TEXTUREDIR"grass.jpg", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	 HMToonTex = SOIL_load_OGL_texture(
		TEXTUREDIR"darkGreen.png", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	HMToonHighTex = SOIL_load_OGL_texture(
		TEXTUREDIR"brown.png", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	heightMap->SetTexture(heightMapTex);
	heightMap->SetHighgroundTex(heightMapHighTex);

	heightMap->SetBumpMap(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren redsDOT3.jpg", SOIL_LOAD_AUTO, 
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	if (!heightMapTex ||
		!heightMapHighTex ||
		!HMToonTex ||
		!HMToonHighTex ||
		!heightMap->GetBumpMap())
		return false;

	SetTextureRepeating(heightMapTex, true);
	SetTextureRepeating(heightMapHighTex, true);
	SetTextureRepeating(HMToonHighTex, true);
	SetTextureRepeating(HMToonTex, true);
	SetTextureRepeating(heightMap->GetBumpMap(), true);

	root = new SceneNode();

	hellNode->SetPosition(Vector3(0,500,0));
	hellNode->SetBoundingRadius(100);
	hellNode->SetShader(sceneShader);
	hellNode->SetUpdateShaderFunction([this]{ UpdateCombineSceneShaderMatricesPO(); } );

	heightMapNode = new SceneNode(heightMap);
	heightMapNode->SetBoundingRadius( sqrt( pow(RAW_WIDTH * HEIGHTMAP_X * 0.5f, 2) + pow(RAW_HEIGHT * HEIGHTMAP_Z * 0.5f, 2)));
	/*heightMapNode->SetShader(sceneShader);
	heightMapNode->SetUpdateShaderFunction([this]{ UpdateCombineSceneShaderMatricesPO(); } );*/
	heightMapNode->SetShader(heightMapShader);
	heightMapNode->SetUpdateShaderFunction([this]{ UpdateHeightMapShaderPO(); });

	heightMapNode->SetTextureMatrix(Matrix4::Scale(Vector3(2,2,2)));
	heightMapNode->SetSpecularPower(20);
	heightMapNode->SetSpecularFactor(0.2f);

	std::cout << "gl error: " << glGetError() << std::endl;

	lightSource = new SceneNode(sphere);
	lightSource->SetPosition(light->GetPosition());
	lightSource->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));
	lightSource->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	lightSource->SetBoundingRadius(100.0f);
	lightSource->SetShader(passThrough);

	root->AddChild(hellNode);
	root->AddChild(heightMapNode);
	root->AddChild(lightSource);

	std::cout << "gl error: " << glGetError() << std::endl;

	return true;
};

void Renderer::DeleteSceneObjects(){
	delete hellData;
	delete root;

	delete quad;
	delete sphere;
}

void Renderer::UpdateSceneObjects(float msec){

	//light->SetPosition(Vector3(2000.0f, 5000.0f, 0)); //MIDDAY

	/*light->SetPosition(Vector3(2000.0f * cos(timeOfDay), 5000.0f * cos(timeOfDay), 5000.0f * sin(timeOfDay)));*/
	light->SetPosition(Vector3(2000.0f * sin(timeOfDay), 5000.0f * sin(timeOfDay), 5000.0f * cos(timeOfDay)));
	
	light->SetRadius(max(6000.0f +  55000.0f * sin(timeOfDay), 0.0f));

	lightSource->SetPosition(light->GetPosition());
	//lightSource->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));
		/*Matrix4::Scale(Vector3(100.0f, 100.0f, 100.0f)));*/

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

	root->Update(msec);
}

void Renderer::BuildNodeLists(SceneNode* from, const Vector3& viewPos, bool transparents){
	if (frameFrustum.InsideFrustum(*from)){
		Vector3 dir = from->GetWorldTransform().GetPositionVector() -
			viewPos;

		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f){
			if (transparents) transparentNodes.push_back(from);
		} else nodeList.push_back(from);

	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart();
		i != from->GetChildIteratorEnd(); ++i){
			BuildNodeLists((*i), viewPos, transparents);
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

void Renderer::ClearNodeLists(){
	transparentNodes.clear();
	nodeList.clear();
}

void Renderer::UpdateHeightMapShaderPF(){
	SetCurrentShader(heightMapShader);

	//upload our cameras position
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());

	SetShaderLight(*light);

	//Bind our depth texture from our shadow FBO to texture unit 2
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"highGroundTex"), 3);
}

void Renderer::UpdateHeightMapShaderPO(){

	Matrix4 tempMatrix = shadowVPMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"shadowVPMatrix"),1,false, tempMatrix.values);

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"highestHeight"), 300);
};