#include "Renderer.h"

bool Renderer::InitShadowBuffers(){
	sceneShader = new Shader(SHADERDIR"shadowSceneVert.glsl",
		SHADERDIR"shadowSceneBumpFrag.glsl");

	shadowShader = new Shader(SHADERDIR"shadowVert.glsl", SHADERDIR"shadowFrag.glsl");

	if (!sceneShader->LinkProgram() || !shadowShader->LinkProgram())
		return false;

	//Update the shaders uniforms that will not be changed throughout the program.
	SetCurrentShader(sceneShader);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"bumpTex"), 1);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"shadowTex"), 2);

	//Generate and bind a texture for our shadow FBO
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	shadowVPMatrix = Matrix4::GetIdentitiy();

	//Remove all filtering
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Set the textures size and bits to use
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	//Tell gl the compare mode; gl compare R to texture (depth value of the texture)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
		GL_COMPARE_R_TO_TEXTURE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//Generate and bind a frame buffer
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	//Bind the shadow texture to the frame buffer in the depth part
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, shadowTex, 0);

	//Tell gl we dont want colours drawn to this frame buffer
	glDrawBuffer(GL_NONE);

	//Unbind the frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void Renderer::DeleteShadowBuffers(){
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);

	delete sceneShader;
	delete shadowShader;
}

void Renderer::DrawShadowScene(){

	//Draw to our shadow framebuffer (with shadow texture)
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	//Clear the old shadow texture
	glClear(GL_DEPTH_BUFFER_BIT);

	//We must disable face culling to prevent light "bleeding"
	glDisable(GL_CULL_FACE); 

	//Set our viewport to be the size of the shadow texture
	glViewport(0,0,SHADOWSIZE,SHADOWSIZE);

	//Turn off colour writing!
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	SetCurrentShader(shadowShader);

	//Create a view matrix from the light position facing the origin
	viewMatrix = Matrix4::BuildViewMatrix(
		light->GetPosition(), Vector3(0,0,0));

	//TODO: Reevaluate this!
	/*projMatrix = Matrix4::Perspective(2900.0f + 2000 * abs(cos(timeOfDay)), 7100.0f - 2000 * abs(cos(timeOfDay)),
		(float) width / (float) height, 45.0f);*/

	//TODO: This seems to work fine... maybe make it a constant?
	projMatrix = Matrix4::Perspective(2900.0f, 7100.0f,
		(float) width / (float) height, 45.0f);

	//Stores the shadows vp matrix multiplied by the bias matrix
	//to keep coordinates in clip space range
	shadowVPMatrix = biasMatrix*(projMatrix*viewMatrix);

	//Draw the quad and the hellknight
	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	BuildNodeLists(root, light->GetPosition());
	SortNodeLists();

	//Only draw the opaque nodes
	for (auto itr = nodeList.begin(); itr != nodeList.end(); ++itr){
		SetCurrentShader(shadowShader);
		(*itr)->Draw(*this, false);
		if (debug) if ((*itr)->GetMesh()) objectsShadowed++;
	}

	ClearNodeLists();

	//Disable our shader, turn colour writes back on and set our view port back to
	//our window size
	glUseProgram(0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0,0,width, height);
	glEnable(GL_CULL_FACE);

	//Unbind our shadow frame buffer too
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Renderer::UpdateCombineSceneShaderMatricesPF(){
	SetCurrentShader(sceneShader);

	//upload our cameras position
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());

	SetShaderLight(*light);

	//Bind our depth texture from our shadow FBO to texture unit 2
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
}

void Renderer::UpdateCombineSceneShaderMatricesPO(){

	textureMatrix.ToIdentity();

	Matrix4 tempMatrix = shadowVPMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"shadowVPMatrix"),1,false, tempMatrix.values);
}
