#include "Renderer.h"

bool Renderer::InitPostProcess(){
	processShader = new Shader(SHADERDIR"TexturedVertex.glsl",
		SHADERDIR"processFrag.glsl");

	if (!processShader->LinkProgram())
		return false;

	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	//Set the texture to clamp on both axis, and use no filtering
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//BPP = Bits per pixel
	//Packed format the texture so there are 24 (bpp) for depth and 8 (bpp) for stencil
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	//And our colour textures
	for (int i=0; i<2; ++i){

		//Generate and bind our texture
		glGenTextures(1, &bufferColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[i]);

		//Turn off filtering and have them clamped
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		//Set the format of the texture to be 8 bits per channel of RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glGenFramebuffers(1, &bufferFBO); // We render the scene into this
	glGenFramebuffers(1, &processFBO); //And do post processing in this

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);	//Bind our bufferFBO object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, bufferDepthTex, 0);	

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, bufferColourTex[0], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ||
		!bufferDepthTex || !bufferColourTex[0]){
		return false;
	}
	//Unbind and enable depth testing
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void Renderer::DrawPostProcess(){

	//Bind our processing FBO and attach bufferColourTex[1] to it
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, bufferColourTex[1], 0);

	//Clear it
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//Set up orthographic rendering using our process shader
	SetCurrentShader(processShader);
	projMatrix = Matrix4::Orthographic(-1,1,1,-1,-1,1);
	viewMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	//Disable our depth test
	glDisable(GL_DEPTH_TEST);

	//upload the pixel size uniform
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"),
		1.0f / width, 1.0f / height);

	for (int i=0; i < POST_PASSES; ++i){
		//Bind our buffer colortex[1] to it as a colour attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, bufferColourTex[1], 0);

		glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
			"isVertical"), 0);

		//Bind colourTex[0] as a texture
		quad->SetTexture(bufferColourTex[0]);

		//Draw the scene and blur horizontally
		quad->Draw();

		//Swap the buffers round between being textures and colour buffer
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
			"isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, bufferColourTex[0], 0);

		quad->SetTexture(bufferColourTex[1]);

		//Draw the scene and blur vertically
		quad->Draw();
	}

	//Post processing complete, unbind FBO and shader and re-enable depth testing
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);

	glEnable(GL_DEPTH_TEST);
}

void Renderer::PresentScene(){
	//We draw the final result to the default framebuffer (the back buffer)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//We must render the final texture orthographically in front of the screen
	SetCurrentShader(passThrough);
	projMatrix = Matrix4::Orthographic(-1,1,1,-1,-1,1);
	viewMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	//The results of post processing is held in bufferColourTex[0]
	quad->SetTexture(bufferColourTex[0]);
	quad->Draw();

	glUseProgram(0);


}

void Renderer::DeletePostProcess(){
	delete processShader;

	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(2, bufferColourTex);

	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);
}