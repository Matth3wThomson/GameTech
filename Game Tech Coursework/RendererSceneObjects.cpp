#include "Renderer.h"

//Initialises all scene objects
bool Renderer::InitSceneObjects(){

	//Create the root node of our scene
	root = new SceneNode();

	heightMapShader = new Shader(SHADERDIR"shadowSceneVert.glsl",
		SHADERDIR"heightMapShadowSceneFrag.glsl");

	if (!heightMapShader->LinkProgram())
		return false;

	SetCurrentShader(heightMapShader);

	//Upload uniforms that wont change
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"bumpTex"), 1);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"shadowTex"), 2);

	heightMap = new HeightMap(TEXTUREDIR"terrain.raw");
	sphere = new OBJMesh(MESHDIR"sphere.obj");

	//Load in all of the textures for the heightMap
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

	heightMap->SetBumpMap(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren redsDOT3.jpg", SOIL_LOAD_AUTO, 
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	if (!heightMapTex ||
		!heightMapHighTex ||
		!HMToonTex ||
		!HMToonHighTex ||
		!heightMap->GetBumpMap())
		return false;

	heightMap->SetTexture(heightMapTex);
	heightMap->SetHighgroundTex(heightMapHighTex);

	SetTextureRepeating(heightMapTex, true);
	SetTextureRepeating(heightMapHighTex, true);
	SetTextureRepeating(HMToonHighTex, true);
	SetTextureRepeating(HMToonTex, true);
	SetTextureRepeating(heightMap->GetBumpMap(), true);

	//Create and set the properties of a sceneNode for the heightmap
	heightMapNode = new SceneNode(heightMap);
	heightMapNode->SetBoundingRadius( sqrt( pow(RAW_WIDTH * HEIGHTMAP_X * 0.5f, 2) + pow(RAW_HEIGHT * HEIGHTMAP_Z * 0.5f, 2)));
	/*heightMapNode->SetShader(sceneShader);
	heightMapNode->SetUpdateShaderFunction([this]{ UpdateCombineSceneShaderMatricesPO(); } );*/
	heightMapNode->SetShader(heightMapShader);
	heightMapNode->SetShaderUpdateFunc([this]{ UpdateHeightMapShaderPO(); });

	//heightMapNode->SetTextureMatrix(Matrix4::Scale(Vector3(2,2,2)));
	//heightMapNode->SetSpecularPower(20);
	//heightMapNode->SetSpecularFactor(0.2f);
	sunShader = new Shader(SHADERDIR"basicVertex.glsl",
		SHADERDIR"colourWhiteFrag.glsl");

	if (!sunShader->LinkProgram())
		return false;

	lightSource = new SceneNode(sphere);
	lightSource->SetTransform(Matrix4::Translation(light->GetPosition()));
	lightSource->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));
	lightSource->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	lightSource->SetBoundingRadius(100.0f);
	lightSource->SetShader(sunShader);

	root->AddChild(heightMapNode);
	root->AddChild(lightSource);

	return true;
};

void Renderer::DeleteSceneObjects(){
	
}

//Updates the scene objects that need updating, and calls update on the root node
void Renderer::UpdateSceneObjects(float msec){

	/*light->SetPosition(Vector3(0, 5000.0f, 0));
	light->SetRadius(55000.0f);*/

	light->SetPosition(Vector3(2000.0f * sin(timeOfDay), 5000.0f * sin(timeOfDay), 5000.0f * cos(timeOfDay)));
	light->SetRadius(max(10000.0f +  55000.0f * sin(timeOfDay), 0.0f));

	lightSource->SetTransform(Matrix4::Translation(light->GetPosition()));

	root->Update(msec);
}

//Builds the node lists given a view point. Will omit transparents if they are not requested.
//(Useful for rendering a shadow map!)
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

//Sort the node lists by distance from camera
void Renderer::SortNodeLists(){
	std::sort(transparentNodes.begin(),
		transparentNodes.end(),
		SceneNode::CompareByCameraDistance);

	std::sort(nodeList.begin(),
		nodeList.end(),
		SceneNode::CompareByCameraDistance);
}

//Clear the node lists upon request
void Renderer::ClearNodeLists(){
	transparentNodes.clear();
	nodeList.clear();
}

//Heightmap shader update per frame
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

//Object specific heightmap shader updates
void Renderer::UpdateHeightMapShaderPO(){

	Matrix4 tempMatrix = shadowVPMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"shadowVPMatrix"),1,false, tempMatrix.values);

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"highestHeight"), 300);
};