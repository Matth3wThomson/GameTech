#pragma once

#include "..\nclgl\scenenode.h"
#include "..\nclgl\ParticleEmitter.h"


/*
	NOTES:
		-Must be a seperate class as it needs to mess with openGL state in order to
		 draw itself correctly. It also needs a different kind of mesh, which has
		 an update method, and therefore needed to update it!
		 
*/

class ParticleEmitterNode :
	public SceneNode
{
public:
	ParticleEmitterNode(Vector4 colour = Vector4(1,1,1,0.5)):SceneNode(NULL, colour){};
	virtual ~ParticleEmitterNode(void){};

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

		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		//glDepthMask(GL_FALSE);

		pe->Draw();

		/*glDepthMask(GL_TRUE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/
		//glDisable(GL_BLEND);

		glUseProgram(0);
	};

protected:
	ParticleEmitter* pe;
};

