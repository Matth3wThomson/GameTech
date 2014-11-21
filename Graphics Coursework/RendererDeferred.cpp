#include "Renderer.h"

/*
	NOTES:
		-To combine shadows with deferred rendering, we have to do forward lighting
		 calculations, and record the "original scene colour" in order to apply our
		 deferred lights correctly. "Original scene colour" is simply the full diffuse tex
		 of the scene if it were to be illuminated entirely with no specularity
*/

//A set lightRadius
#define LIGHTRADIUS 500

//Easier for looping!
Vector3 lightPositions[5] = {
	Vector3(300, 50, 1500), 
	Vector3(500, 50, 500), 
	Vector3(-1050, 150, 520), 
	Vector3(-900, 350, -600), 
	Vector3(-1000, 190, -1500)
								};

//This method initialises all things for deferred rendering
bool Renderer::InitDeferredRendering(){

	//Make the fires share a texture... dont want to be wasting memory!
	fireParticleTex = SOIL_load_OGL_texture(TEXTUREDIR"particle.tga",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	numPointLights = 5;

	pointLights = new Light[numPointLights];

	//Create our deferred lights and the "fire" emitters to go with them
	for (int i=0; i<numPointLights; ++i){
		pointLights[i].SetColour(Vector4(1,0.5f,0,1));
		pointLights[i].SetRadius(LIGHTRADIUS);
		pointLights[i].SetPosition(Vector3(
			lightPositions[i].x,
			lightPositions[i].y,
			lightPositions[i].z));

		ParticleEmitter* fireEmitter = new ParticleEmitter();
		fireEmitter->SetTexture(fireParticleTex);
		fireEmitter->SetColourVariance(0.5f);
		fireEmitter->SetDefaultColour(Vector4(1,0.5f,0,1));
		fireEmitter->SetDirection(Vector3(0,1,0));
		fireEmitter->SetParticleSpeed(0.2f);
		fireEmitter->SetParticleRate(20.0f);
		fireEmitter->SetLaunchParticles(2);

		ParticleEmitterNode* fire = new ParticleEmitterNode();
		fire->SetParticleEmitter(fireEmitter);
		fire->SetPosition(lightPositions[i]);
		fire->SetBoundingRadius(100);
		fire->SetShader(particleShader);
		/*fire->SetUpdateShaderFunction([this]{UpdateParticleShaderMatricesPO(); });*/
		fire->SetScaleWithParent(false);
		
		root->AddChild(fire);
	}

	//Create the mesh for drawing deferred lights
	icoSphere = new OBJMesh(MESHDIR"ico.obj");

	//Load in the necessary shaders
	combineShader = new Shader(SHADERDIR"combineVert.glsl",
		SHADERDIR"combineFrag.glsl");

	pointLightShader = new Shader(SHADERDIR"pointLightVert.glsl",
		SHADERDIR"pointLightFrag.glsl");

	if (!combineShader->LinkProgram() || !pointLightShader->LinkProgram())
		return false;

	//Here our bufferFBO has already been created, so we
	//only create our pointlightFBO
	glGenFramebuffers(1, &pointLightFBO);

	//We want to generate a new screen texture for it!
	//and add to our bufferFBO a normal texture and an
	// "original scene" texture.
	GenerateScreenTexture(originalSceneTex);
	GenerateScreenTexture(bufferNormalTex);
	GenerateScreenTexture(lightEmissiveTex);
	GenerateScreenTexture(lightSpecularTex);

	//Attach the generate textures to the correct FBOs
	GLenum buffers[3];
	buffers[0] = GL_COLOR_ATTACHMENT0;
	buffers[1] = GL_COLOR_ATTACHMENT1;
	buffers[2] = GL_COLOR_ATTACHMENT2;

	//Just bind the new normals tex from here! And another 
	//texture to represent the "fully lit" scene
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D, bufferNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
		GL_TEXTURE_2D, originalSceneTex, 0);

	glDrawBuffers(3, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != 
		GL_FRAMEBUFFER_COMPLETE)
		return false;

	//Bind our light emissive and specular textures to our pointlight FBO
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, lightEmissiveTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D, lightSpecularTex, 0);
	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
		GL_FRAMEBUFFER_COMPLETE)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void Renderer::UpdateDeferredRendering(float msec){
	//This is to make the fires "flicker". Its only a cheeky oscillation though.
	for (int i=0; i<numPointLights; ++i){
		pointLights[i].SetRadius(LIGHTRADIUS + ((LIGHTRADIUS / 5) * sin(timeOfDay*1000)));
	}
}

void Renderer::DeleteDeferredRendering(){
	glDeleteTextures(1, &fireParticleTex);

	delete[] pointLights;
	delete icoSphere;
	delete combineShader;
	delete pointLightShader;

	glDeleteTextures(1, &originalSceneTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &lightEmissiveTex);
	glDeleteTextures(1, &lightSpecularTex);

	glDeleteFramebuffers(1, &pointLightFBO);

};

void Renderer::DrawPointLights(){

	SetCurrentShader(pointLightShader);

	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);

	//We clear the buffer to black (representing no light)
	glClearColor(0,0,0,1);

	glEnable(GL_CULL_FACE);

	//We only clear the color buffer (because thats all we have attached)
	glClear(GL_COLOR_BUFFER_BIT);

	//When a fragment is lit by more than one light, we want additive blending
	glBlendFunc(GL_ONE, GL_ONE);

	//We bind the normal and depth information calculated in the first pass (of each fragment)
	//to the light shader as a uniform variable
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"depthTex"), 3);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"normTex"), 4);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	//We upload camera pos to the GPU for specularity calculations
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());
	
	//Pixel size is uploaded for ndc space access to textures
	Vector2 pixelSize = Vector2(1.0f / width, 1.0f / height);
	glUniform2fv(glGetUniformLocation(currentShader->GetProgram(),
		"pixelSize"), 1 , (float*)&pixelSize);

	viewMatrix = camera->BuildViewMatrix();
	projMatrix = cameraProjMat;
	textureMatrix.ToIdentity();

	//Draw all of the pointlights in our scene to our lightingFBO
	for (int x=0; x < numPointLights; ++x){
		Light& l = pointLights[x];

		float radius = l.GetRadius();

		modelMatrix = Matrix4::Translation(l.GetPosition()) 
			* Matrix4::Scale(Vector3(radius, radius, radius));

		SetShaderLight(l);

		UpdateShaderMatrices();

		float dist = (l.GetPosition() - camera->GetPosition()).Length();

		if (dist < radius){
			glCullFace(GL_FRONT);
		} else {
			glCullFace(GL_BACK);
		}

		sphere->Draw();
	}

	//We reset everything to normal
	glCullFace(GL_BACK);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

void Renderer::CombineBuffers(){

	//When combining our textures we need to use the processFBO, 
	//as we dont want to override our depth texture we created before!
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, GetDrawTarget(), 0);

	SetCurrentShader(combineShader);

	projMatrix = Matrix4::Orthographic(-1, 1, 1, -1, -1, 1);
	UpdateShaderMatrices();

	//We pass in the forward lit scene, pass-through image as a texture, and the lighting information
	//to calculate the final fragments colours as textures
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 2);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"emissiveTex"), 3);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"specularTex"), 4);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"originalSceneTex"), 5);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, GetLastDrawn());
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, lightEmissiveTex);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, originalSceneTex);

	//We draw our quad orthographically (filling up the entire screen)
	quad->Draw();

	//Let the post process system know that a texture has been drawn to.
	//"Pinged"
	PPDrawn();
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//A simple method for creating screen textures without all the extra fluff!
void Renderer::GenerateScreenTexture(GLuint& into, bool depth){
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	//Turn off filtering (as its a screen texture)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//If a depth texture is required then make it so
	glTexImage2D(GL_TEXTURE_2D, 0, 
		depth ? GL_DEPTH_COMPONENT24: GL_RGBA8,
		width, height, 0,
		depth ? GL_DEPTH_COMPONENT : GL_RGBA,
		GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
}

