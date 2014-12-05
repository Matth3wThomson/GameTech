#include "Renderer.h"

//Initialises all scene objects
bool Renderer::InitSceneObjects(){

	//Create the root node of our scene
	root = new SceneNode();

	return true;
};

void Renderer::DeleteSceneObjects(){
	
}

//Updates the scene objects that need updating, and calls update on the root node
void Renderer::UpdateSceneObjects(float msec){

	light->SetPosition(Vector3(0, 5000.0f, 0));
	light->SetRadius(55000.0f);

	//light->SetPosition(Vector3(2000.0f * sin(timeOfDay), 5000.0f * sin(timeOfDay), 5000.0f * cos(timeOfDay)));
	//light->SetRadius(max(10000.0f +  55000.0f * sin(timeOfDay), 0.0f));

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