#include "Renderer.h"

bool Renderer::InitWater(){
	
	reflectShader = new Shader(SHADERDIR"shadowSceneVert.glsl",
		SHADERDIR"reflectBumpFrag.glsl");

	if (!reflectShader->LinkProgram())
		return false;

	//Upload all uniforms that will not change thoughout the program!
	SetCurrentShader(reflectShader);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"bumpTex"), 1);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
			"shadowTex"), 2);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"cubeTex"), 3);

	waterQuad = Mesh::GenerateQuad();

	waterQuad->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"blue3.png",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	if (!waterQuad->GetTexture())
		return false;

	SetTextureRepeating(waterQuad->GetTexture(), true);

	GLfloat aniso;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

	waterQuad->SetBumpMap(SOIL_load_OGL_texture(TEXTUREDIR"waves.jpg",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	if (!waterQuad->GetBumpMap())
		return false;

	SetTextureRepeating(waterQuad->GetBumpMap(), true);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

	

	waterNode = new SceneNode(waterQuad, Vector4(1,1,1,0.5));
	waterNode->SetPosition(Vector3(-1115.9f,150,-1501.6f));/* *
		Matrix4::Rotation(90, Vector3(1,0,0)));*/

	waterNode->SetModelScale(Vector3(939.3f,554.9f,1));
	waterNode->SetModelRotation(90, Vector3(1,0,0));
		/*Matrix4::Scale(Vector3(939.3f,554.9f,1)));*/
	waterNode->SetBoundingRadius(1000);
	//waterNode->SetColour(Vector4(1,1,1,0.5));

	waterNode->SetShader(reflectShader);
	waterNode->SetUpdateShaderFunction([this](){ UpdateWaterShaderMatricesPO(); } );
	waterNode->SetSpecularPower(50);
	waterNode->SetSpecularFactor(0.33f);

	root->AddChild(waterNode);

	return true;
}

void Renderer::DeleteWater(){
	delete reflectShader;
}

//Frame + object specific uploads.
void Renderer::UpdateWaterShaderMatricesPO(){

	Matrix4 tempMatrix = shadowVPMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
			"shadowVPMatrix"),1,false, tempMatrix.values);

	//TODO: Make this not frame specific!
	rotation += 0.05f;

	textureMatrix = Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) *
		Matrix4::Rotation(rotation, Vector3(0.0f, 0.0f, 1.0f));

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
}

void Renderer::UpdateWaterShaderMatricesPF(){
	SetCurrentShader(reflectShader);

	SetShaderLight(*light);

	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());
}