#include "Renderer.h"

//Initialises the water in the scene
bool Renderer::InitWater(){
	return true;
}

void Renderer::DeleteWater(){

}

void Renderer::UpdateWater(float msec){
	//To create the illusion of the water "moving" we must rotate its texture matrix
	rotation += 0.005f * msec;
}

//Per frame specific water shader uploads.
void Renderer::UpdateWaterShaderMatricesPO(){

	Matrix4 tempMatrix = shadowVPMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
			"shadowVPMatrix"),1,false, tempMatrix.values);
	

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
}

//Per object specific water shader uploads.
void Renderer::UpdateWaterShaderMatricesPF(){
	SetCurrentShader(reflectShader);

	SetShaderLight(*light);

	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());
}