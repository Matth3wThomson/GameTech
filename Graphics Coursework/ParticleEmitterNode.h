#pragma once

#include "..\nclgl\scenenode.h"
#include "..\nclgl\ParticleEmitter.h"


/*
	NOTES:
		-Must be a seperate class as it needs a different kind of mesh, which has
		 an update method, and therefore needed to update it! It also needs object specific
		 shader uploads.
		-Get Mesh was overriden as the program uses it to determine if a node is drawable.
		 
*/

class ParticleEmitterNode :
	public SceneNode
{
public:
	ParticleEmitterNode(Vector4 colour = Vector4(1,1,1,0.5)):SceneNode(NULL, colour){
		pe = NULL;
	};

	virtual ~ParticleEmitterNode(void){
		delete pe;
	};

	void SetParticleEmitter(ParticleEmitter* pe){ this->pe = pe; };
	ParticleEmitter* GetParticleEmitter() const { return pe; };
	virtual Mesh* GetMesh() const { return pe; }

	virtual void Update(float msec){
		pe->Update(msec);
		SceneNode::Update(msec);
	};

	virtual void Draw(OGLRenderer& r, const bool useShader = true){
		r.SetCurrentShader(shader);

		glUniform1f(glGetUniformLocation(r.currentShader->GetProgram(), "particleSize"), pe->GetParticleSize());
		
		r.modelMatrix = this->GetWorldTransform();

		r.UpdateShaderMatrices();

		pe->Draw();

		glUseProgram(0);
	};

protected:
	ParticleEmitter* pe;
};

