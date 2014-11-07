#include "SceneNode.h"

SceneNode::SceneNode(Mesh* mesh, Vector4 colour){
	this->mesh = mesh;
	this->colour = colour;

	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;

	parent = NULL;
	modelScale = Vector3(1,1,1);
}

SceneNode::~SceneNode(void)
{
	for (unsigned int i=0; i<children.size(); ++i){
		delete children[i];
	}
}

void SceneNode::AddChild(SceneNode* s){
	children.push_back(s);
	s->parent = this;
}

//UNTESTED
void SceneNode::RemoveChild(SceneNode* s){
	for (auto itr = children.begin(); itr != children.end(); ++itr)
		if (*itr = s) children.erase(itr);
	
}

void SceneNode::Draw(const OGLRenderer& r){
	if (mesh) mesh->Draw();
}

void SceneNode::Update(float msec){
	if (parent) worldTransform = parent->worldTransform * transform;
	else worldTransform = transform;

	for (vector<SceneNode*>::iterator i = children.begin();
		i!= children.end(); ++i){
			(*i)->Update(msec);
	}
}
