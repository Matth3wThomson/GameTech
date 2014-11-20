#include "SceneNode.h"

SceneNode::SceneNode(Mesh* mesh, Vector4 colour){
	this->mesh = mesh;
	this->colour = colour;
	this->shader = NULL;
	this->updateShaderFunction = []{};

	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;

	textureMatrix.ToIdentity();

	parent = NULL;
	scaleWithParent = true;
	modelScale = Vector3(1,1,1);
	angle = 0;
	rotationAxis = Vector3(0,1,0);

	specularPower = 33;
	specularFactor = 0.3f;
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

		r.textureMatrix = textureMatrix;

		r.modelMatrix = worldTransform *  Matrix4::Scale(worldScale * modelScale);

		glUniform1i(glGetUniformLocation(r.currentShader->GetProgram(),
			"specularPower"), specularPower);

		glUniform1f(glGetUniformLocation(r.currentShader->GetProgram(),
			"specFactorMod"), specularFactor);

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

		/*if (scaleWithParent){*/
			worldScale = parent->modelScale * parent->worldScale;

			//We need to transform ourselves relative to our parent's scale, to maintain
			//correct distance from it. For example, if a person gets larger, we want the arm
			//still to be connected to the shoulder!
			transform =  Matrix4::Translation(position * worldScale) * GetRotationMatrix();

		/*} else {
			worldScale = Vector3(1,1,1);
			transform = Matrix4::Translation(position);
		}*/

		if (!scaleWithParent)
			worldScale = Vector3(1,1,1);

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
