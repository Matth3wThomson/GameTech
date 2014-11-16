#include "Renderer.h"

bool Renderer::InitSceneObjects(){
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

	heightMap->SetTexture(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren reds.jpg", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	heightMap->SetBumpMap(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren redsDOT3.jpg", SOIL_LOAD_AUTO, 
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	std::cout << "gl error: " << glGetError() << std::endl;

	if (!heightMap->GetTexture() || !heightMap->GetBumpMap())
		return false;

	std::cout << "gl error: " << glGetError() << std::endl;

	SetTextureRepeating(heightMap->GetTexture(), true);
	SetTextureRepeating(heightMap->GetBumpMap(), true);

	std::cout << "gl error: " << glGetError() << std::endl;

	root = new SceneNode();

	hellNode->SetTransform(Matrix4::Translation(Vector3(0,500,0)));
	hellNode->SetBoundingRadius(100);
	hellNode->SetShader(sceneShader);
	hellNode->SetUpdateShaderFunction([this]{ UpdateCombineSceneShaderMatricesPO(hellNode); } );

	heightMapNode = new SceneNode(heightMap);
	heightMapNode->SetBoundingRadius( sqrt( pow(RAW_WIDTH * HEIGHTMAP_X * 0.5f, 2) + pow(RAW_HEIGHT * HEIGHTMAP_Z * 0.5f, 2)));
	heightMapNode->SetShader(sceneShader);
	heightMapNode->SetUpdateShaderFunction([this]{ UpdateCombineSceneShaderMatricesPO(heightMapNode); } );

	std::cout << "gl error: " << glGetError() << std::endl;

	lightSource = new SceneNode(sphere);
	lightSource->SetTransform(Matrix4::Translation(light->GetPosition()) *
		Matrix4::Scale(Vector3(100.0f, 100.0f, 100.0f)));
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
	light->SetPosition(Vector3(2000.0f * cos(movementVar), 5000.0f * cos(movementVar), 5000.0f * sin(movementVar)));
	
	light->SetRadius(max(6000.0f +  55000.0f * cos(movementVar), 0.0));

	lightSource->SetTransform(Matrix4::Translation(light->GetPosition()) *
		Matrix4::Scale(Vector3(100.0f, 100.0f, 100.0f)));

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

/*if (light->GetPosition().y < -500) light->SetRadius(sin(movementVar) * 8000.0f);
	else light->SetRadius(55000.0f);*/
	/*if (light->GetPosition().y > -500) light->SetRadius(9000.0f + 500000.0f * cos(movementVar));
	else light->SetRadius(1000.0f);*/

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

void Renderer::ClearNodeLists(){
	transparentNodes.clear();
	nodeList.clear();
}