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

Vector3 SceneNode::GetWorldScale() const {
	const SceneNode* parent = this->parent;
	Vector3 overallScale = this->modelScale;

	while (parent){
		overallScale = overallScale * parent->modelScale;
		parent = parent->parent;
	}

	return overallScale;
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
	//if (mesh) mesh->Draw();
	if (mesh && shader){
		if (useShader){
			r.SetCurrentShader(shader);
			//NEW
			updateShaderFunction();
		}

		//TODO: Change this to world transform!
		r.modelMatrix = GetWorldTransform() * GetRotationMatrix() * Matrix4::Scale(GetModelScale());
		r.UpdateShaderMatrices();

		mesh->Draw();

		glUseProgram(0);
	}
}

void SceneNode::Update(float msec){
	if (parent) worldTransform = parent->worldTransform * transform;
	else worldTransform = transform;

	for (vector<SceneNode*>::iterator i = children.begin();
		i!= children.end(); ++i){
			(*i)->Update(msec);
	}
}
