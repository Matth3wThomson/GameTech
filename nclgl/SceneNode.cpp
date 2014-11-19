#include "SceneNode.h"

//TODO: With scale stored it is possible to offset certain objects if their mesh isnt origin centered locally

SceneNode::SceneNode(Mesh* mesh, Vector4 colour){
	this->mesh = mesh;
	this->colour = colour;
	this->shader = NULL;
	this->updateShaderFunction = []{};

	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;

	parent = NULL;
	modelScale = Vector3(1,1,1);
	angle = 0;
	rotationAxis = Vector3(0,1,0);
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
	for (auto itr = children.begin(); itr != children.end(); )
		if ((*itr) == s){
			itr = children.erase(itr);
		} else {
			itr++;
		}
}

void SceneNode::Draw(OGLRenderer& r, const bool useShader){

	if (mesh && shader){
		if (useShader) r.SetCurrentShader(shader);
		

		
		r.modelMatrix = worldTransform *  Matrix4::Scale(worldScale * modelScale);

		if (useShader) updateShaderFunction();

		r.UpdateShaderMatrices();

		mesh->Draw();

		glUseProgram(0);
	}
}

void SceneNode::Update(float msec){
	//If this node has a parent
	if (parent){

		//We need to work out its parents relative scale. That is, the
		//scale of its parent
		worldScale = parent->modelScale * parent->worldScale;

		//We need to transform ourselves relative to our parent's scale, to maintain
		//correct distance from it. For example, if a person gets larger, we want the arm
		//still to be connected to the shoulder!
		transform =  Matrix4::Translation(position * worldScale) * GetRotationMatrix();

		//Finally, our world transform is our parents world transform multiplied by ours.
		worldTransform = parent->worldTransform * transform;
		
	} else {

		//We are the parent, so we transform based on noone!
		//We are scaled relative to ourself
		worldTransform = Matrix4::Translation(position);
		transform = worldTransform;
		worldScale = Vector3(1,1,1);
	}

	for (vector<SceneNode*>::iterator i = children.begin();
		i!= children.end(); ++i){
			(*i)->Update(msec);
	}
}
