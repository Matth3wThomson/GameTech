#include "Renderer.h"

void Renderer::InitSceneObjects(){
	hellData = new MD5FileData(MESHDIR"hellknight.md5mesh");
	hellNode = new MD5Node(*hellData);

	hellData->AddAnim(MESHDIR"idle2.md5anim");
	hellData->AddAnim(MESHDIR"attack2.md5anim");
	hellData->AddAnim(MESHDIR"pain1.md5anim");
	hellData->AddAnim(MESHDIR"roar1.md5anim");
	hellData->AddAnim(MESHDIR"stand.md5anim");
	hellData->AddAnim(MESHDIR"walk7.md5anim");

	heightMap = new HeightMap(TEXTUREDIR"terrain.raw");

	heightMap->SetTexture(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren reds.jpg", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	heightMap->SetBumpMap(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren redsDOT3.jpg", SOIL_LOAD_AUTO, 
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	SetTextureRepeating(heightMap->GetTexture(), true);
	SetTextureRepeating(heightMap->GetBumpMap(), true);

	root = new SceneNode();

	/*SceneNode**/ quadNode = new SceneNode(quad);
	quadNode->SetTransform(Matrix4::Rotation(90, Vector3(1,0,0)) *
		Matrix4::Scale(Vector3(450, 450, 1)));
	quadNode->SetBoundingRadius(620.0f);

	hellNode->SetTransform(Matrix4::Translation(Vector3(0,200,0)));
	hellNode->SetBoundingRadius(100);

	heightMapNode = new SceneNode(heightMap);
	//heightMapNode->SetTransform(Matrix4::Translation(Vector3(-(RAW_WIDTH * HEIGHTMAP_X * 0.5f), 0, -(RAW_HEIGHT * HEIGHTMAP_Z * 0.5f))));
	heightMapNode->SetBoundingRadius( sqrt( pow(RAW_WIDTH * HEIGHTMAP_X * 0.5, 2) + pow(RAW_HEIGHT * HEIGHTMAP_Z * 0.5, 2)));

	//root->AddChild(quadNode);
	root->AddChild(hellNode);
	root->AddChild(heightMapNode);

};

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

		if (drawBounds){
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
				"modelMatrix"),	1,false, (float*) &(modelMatrix *
				Matrix4::Scale(
				Vector3(n->GetBoundingRadius(),
				n->GetBoundingRadius(),
				n->GetBoundingRadius()))));
			sphere->Draw();
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}