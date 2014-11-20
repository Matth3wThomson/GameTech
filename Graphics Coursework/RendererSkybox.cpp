#include "Renderer.h"


//TODO: Possibly use 2 skyboxes?
bool Renderer::InitSkybox(){
	skyboxShader = new Shader(SHADERDIR"skyboxVertex.glsl",
		SHADERDIR"skyboxFragment.glsl");

	if (!skyboxShader->LinkProgram())
		return false;

	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg",
		TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!cubeMap)
		return false;

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	skyboxLight = 1.0f;

	return true;

}

void Renderer::UpdateSkybox(float msec){

	skyboxLight = min(0.1 + sin(min(timeOfDay, PI)), 1.0f);

	//TODO: Remove
	/*std::cout << "timeofday: " << timeOfDay << std::endl;
	std::cout << skyboxLight << std::endl;*/
}

void Renderer::DrawSkybox(){

	//TODO: Reduce overdraw of skybox?
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	SetCurrentShader(skyboxShader);

	//No need to change model matrix
	//TODO: This is being called twice!
	modelMatrix = Matrix4::GetIdentitiy();
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = cameraProjMat;

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"blend"), skyboxLight);

	UpdateShaderMatrices();

	quad->Draw();

	glUseProgram(0);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
}

void Renderer::DeleteSkybox(){

	delete skyboxShader;
}
