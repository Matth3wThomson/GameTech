#include "Renderer.h"

//Initialises all things post process!
bool Renderer::InitPostProcess(){
	toDrawTo = 0;
	sobel = false;
	sobelDepth = false;
	quantizeCol = false;
	dubVis = false;
	blur = false;
	bloom  = false;
	fog = false;

	//Load in all of our post processing shaders
	blurShader = new Shader(SHADERDIR"TexturedVertex.glsl",
		SHADERDIR"blurFrag.glsl");

	doubVisShader = new Shader(SHADERDIR"TexturedVertex.glsl",
		SHADERDIR"doubleVisionFrag.glsl");

	sobelShader = new Shader(SHADERDIR"TexturedVertex.glsl",
		SHADERDIR"sobelFrag.glsl");

	sobelDepthShader = new Shader(SHADERDIR"TexturedVertex.glsl",
		SHADERDIR"sobelDepthFrag.glsl");

	quantizeColShader = new Shader(SHADERDIR"TexturedVertex.glsl",
		SHADERDIR"quantizeFrag.glsl");

	fogShader = new Shader(SHADERDIR"TexturedVertex.glsl",
		SHADERDIR"fogFrag.glsl");

	bloomShader = new Shader(SHADERDIR"TexturedVertex.glsl",
		SHADERDIR"bloomFrag.glsl");

	if (!blurShader->LinkProgram() ||
		!sobelDepthShader->LinkProgram() ||
		!quantizeColShader->LinkProgram() ||
		!sobelShader->LinkProgram() ||
		!fogShader->LinkProgram() ||
		!doubVisShader->LinkProgram() ||
		!bloomShader->LinkProgram())
		return false;

	//Generate our frame buffers
	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	//Set the texture to clamp on both axis, and use no filtering
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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

	//Generate our frame buffers
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

void Renderer::DeletePostProcess(){
	delete blurShader;
	delete doubVisShader;
	delete sobelShader;
	delete sobelDepthShader;
	delete quantizeColShader;
	delete fogShader;
	delete bloomShader;
	

	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &originalSceneTex);
	glDeleteTextures(2, bufferColourTex);

	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);
}

//Toggles of our post process effects. msec is not necessary here at all,
//but its passed for consitencys sake!
void Renderer::UpdatePostProcess(float msec){

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_5))
		fog = !fog;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_6))
		sobelDepth = !sobelDepth;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_7))
		quantizeCol = !quantizeCol;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_8))
		sobel = !sobel;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_9))
		dubVis = !dubVis;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_0))
		blur = !blur;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_MINUS))
		bloom = !bloom;
}

//Draw our post process effects using ping pong
void Renderer::DrawPostProcess(){
	glDisable(GL_DEPTH_TEST);
	SetupPPMatrices();
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);

	if (sobel) Sobel();
	if (sobelDepth) SobelDepth();
	if (quantizeCol) QuantizeCol();
	if (fog) Fog();
	if (bloom) Bloom();
	if (dubVis) DoubleVision();
	if (blur) Blur();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);

	glEnable(GL_DEPTH_TEST);
}

void Renderer::SetupPPMatrices(){
	projMatrix = Matrix4::Orthographic(-1,1,1,-1,-1,1);
	viewMatrix.ToIdentity();
	modelMatrix.ToIdentity();
}

//Performs a gaussian blur on the FBO
void Renderer::Blur(){
	//Bind our processing FBO and attach bufferColourTex[1] to it
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, GetDrawTarget(), 0);

	//Clear it
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	SetCurrentShader(blurShader);
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"),
		1.0f / width, 1.0f / height);
	UpdateShaderMatrices();

	//One pass is actually 2!
	for (int i=0; i < BLUR_PASSES*2; ++i){
		//Bind our buffer colortex[1] to it as a colour attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, GetDrawTarget(), 0);

		glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
			"isVertical"), 0);

		quad->SetTexture(GetLastDrawn());

		//Draw the scene and blur horizontally
		quad->Draw();
		PPDrawn();

		//Swap the buffers round between being textures and colour buffer
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
			"isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, GetDrawTarget(), 0);

		quad->SetTexture(GetLastDrawn());

		//Draw the scene and blur vertically
		quad->Draw();
		PPDrawn();
	}
}

//Performs sobel edge detection on a the colour buffer
void Renderer::Sobel(){
	//Bind our processing FBO and attach the next buffer to be drawn to to it
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, GetDrawTarget(), 0);

	//Clear it
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	SetCurrentShader(sobelShader);
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"),
		1.0f / width, 1.0f / height);
	UpdateShaderMatrices();

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"threshold"), 0.8f);

	quad->SetTexture(GetLastDrawn());

	quad->Draw();
	PPDrawn();
}

//Performs sobel edge detection on the contents of a linearised depth buffer
void Renderer::SobelDepth(){
	//Bind our processing FBO and attach the next buffer to be drawn to to it
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, GetDrawTarget(), 0);

	//Clear it
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	SetCurrentShader(sobelDepthShader);
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"),
		1.0f / width, 1.0f / height);
	UpdateShaderMatrices();

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"threshold"), 0.2f);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"depthTex"), 2);

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"nearPlane"), 1.0);

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"farPlane"), 15000.0f);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	quad->SetTexture(GetLastDrawn());

	quad->Draw();
	PPDrawn();
}

//Performs colour quantization by calculating light intensity
void Renderer::QuantizeCol(){
	//Bind our processing FBO and attach the next buffer to be drawn to to it
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, GetDrawTarget(), 0);

	//Clear it
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	SetCurrentShader(quantizeColShader);
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"),
		1.0f / width, 1.0f / height);
	UpdateShaderMatrices();

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"threshold"), 0.2f);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"depthTex"), 2);

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"nearPlane"), 1.0);

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"farPlane"), 15000.0f);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	quad->SetTexture(GetLastDrawn());

	quad->Draw();
	PPDrawn();
}

//Performs a fog effect based upon a linearized depth buffer
void Renderer::Fog(){
	//Bind our processing FBO and attach the next buffer to be drawn to to it
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, GetDrawTarget(), 0);

	//Clear it
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	SetCurrentShader(fogShader);
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"),
		1.0f / width, 1.0f / height);
	UpdateShaderMatrices();

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"threshold"), 0.2f);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"depthTex"), 2);

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"nearPlane"), 1.0);

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),
		"farPlane"), 15000.0f);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	quad->SetTexture(GetLastDrawn());

	quad->Draw();
	PPDrawn();
}

//Performs a double vision effect on the screen
void Renderer::DoubleVision(){
	//Bind our processing FBO and attach the next buffer to be drawn to to it
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, GetDrawTarget(), 0);

	//Clear it
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	SetCurrentShader(doubVisShader);
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"),
		1.0f / width, 1.0f / height);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"pixelOffset"), 40);

	UpdateShaderMatrices();

	quad->SetTexture(GetLastDrawn());

	quad->Draw();
	PPDrawn();

}

//Blooms each fragment based on its local fragments
void Renderer::Bloom(){
	//Bind our processing FBO and attach the next buffer to be drawn to to it
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, GetDrawTarget(), 0);

	//Clear it
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	SetCurrentShader(bloomShader);
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"),
		1.0f / width, 1.0f / height);
	UpdateShaderMatrices();

	quad->SetTexture(GetLastDrawn());

	quad->Draw();
	PPDrawn();

}

//Presents which ever scene was the last to be drawn to to the back buffer
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
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD4)){
		quad->SetTexture(lightEmissiveTex);
	} else if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD5)) {
		quad->SetTexture(lightSpecularTex);
	} else if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD6)) {
		quad->SetTexture(bufferNormalTex);
	} else {
		quad->SetTexture(GetLastDrawn());

	}

	quad->Draw();

	glUseProgram(0);

}