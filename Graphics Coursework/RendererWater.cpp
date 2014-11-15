#include "Renderer.h"

bool Renderer::InitWater(){
	waterTex = 0;
	//TODO: Get bump mapping in!
	/*reflectShader = new Shader(SHADERDIR"PerPixelVertex.glsl",
	SHADERDIR"reflectFragment.glsl");*/

	/*reflectShader = new Shader(SHADERDIR"BumpVertex.glsl",
	SHADERDIR"reflectBumpFrag.glsl");*/
	reflectShader = new Shader(SHADERDIR"shadowSceneVert.glsl",
		SHADERDIR"reflectBumpFrag.glsl");

	if (!reflectShader->LinkProgram())
		return false;

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"blue2.jpg",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	GLfloat aniso;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

	waterBump = SOIL_load_OGL_texture(TEXTUREDIR"waves.jpg",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

	if (!waterTex)
		return false;

	if (!waterBump)
		return false;

	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(waterBump, true);

	return true;
}

void Renderer::DeleteWater(){
	delete reflectShader;
}

void Renderer::DrawWater(bool shadowMap){

	//TODO: Test changing order of scale+rotate
	SetCurrentShader(reflectShader);

	SetShaderLight(*light);
	quad->SetTexture(waterTex);
	quad->SetBumpMap(waterBump);

	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"bumpTex"), 1);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"cubeTex"), 2);

	if (!shadowMap){
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
			"shadowTex"), 3);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shadowTex);

	}

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	//TODO: NOT TIME SPECIFIC
	rotation += 0.05f;

	modelMatrix = Matrix4::Translation(Vector3(-1115.9f,150,-1501.6f)) *
		Matrix4::Rotation(90, Vector3(1,0,0)) *
		Matrix4::Scale(Vector3(939.3f,554.9f,1));

	textureMatrix = Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) *
		Matrix4::Rotation(rotation, Vector3(0.0f, 0.0f, 1.0f));

	UpdateShaderMatrices();

	if (!shadowMap){
		Matrix4 tempMatrix = shadowVPMatrix * modelMatrix;

		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
			"shadowVPMatrix"),1,false, tempMatrix.values);
	} else {
		glDisable(GL_CULL_FACE);
	}

	quad->Draw();
	quad->SetBumpMap(0);

	if (shadowMap)
		glEnable(GL_CULL_FACE);

	glUseProgram(0);
}