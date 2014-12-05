#include "Renderer.h"

//Initializes the generic weather particle emitters, and particle shaders
bool Renderer::InitParticles(){
	particleShader = new Shader(SHADERDIR"basicVertex.glsl", 
		SHADERDIR"colourFragment.glsl", SHADERDIR"particleEmitGeom.glsl");

	if (!particleShader->LinkProgram()) return false;

	SetCurrentShader(particleShader);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);


	//We want a snow particle emitter that drops snowflakes slowly
	snow = new ParticleEmitter(TEXTUREDIR"snowflake.png");

	snow->SetParticleRate(25.0f);
	snow->SetParticleSize(5.0f);
	snow->SetParticleVariance(0.0f);
	snow->SetLaunchParticles(16);
	snow->SetParticleLifetime(20000.0f);
	snow->SetParticleSpeed(0.1f);
	snow->SetDirection(Vector3(0,-1,0));
	snow->SetColourVariance(0.0f);
	snow->SetParticlePositionVariance(HEIGHTMAP_X * RAW_WIDTH * 0.5f);
	snow->SetYOffset(2000.0f);
	snow->SetFadeOverTime(false);
	
	//We want a rain particle emitter that drops rain drops quickly!
	rain = new ParticleEmitter(TEXTUREDIR"Raindrop.png");

	rain->SetParticleRate(25.0f);
	rain->SetParticleSize(20.0f);
	rain->SetParticleVariance(0.0f);
	rain->SetLaunchParticles(16);
	rain->SetParticleLifetime(2000.0f);
	rain->SetParticleSpeed(1.0f);
	rain->SetDirection(Vector3(0,-1,0));
	rain->SetColourVariance(0.0f);
	rain->SetParticlePositionVariance(HEIGHTMAP_X * RAW_WIDTH * 0.5f);
	rain->SetYOffset(2000.0f);
	rain->SetFadeOverTime(false);

	//Create these as sceneNodes so that they can be culled when not in view
	rainNode = new ParticleEmitterNode();
	rainNode->SetParticleEmitter(rain);
	//rainNode->SetShader(particleShader);
//	rainNode->SetPosition(Vector3(0,0,0));
	rainNode->SetBoundingRadius(
		sqrt(pow((HEIGHTMAP_X * RAW_WIDTH * 0.5f),2) + pow((HEIGHTMAP_Z * RAW_HEIGHT * 0.5f),2)));

	snowNode = new ParticleEmitterNode();
	snowNode->SetParticleEmitter(snow);
//	snowNode->SetShader(particleShader);
	//snowNode->SetPosition(Vector3(0,0,0));
	snowNode->SetBoundingRadius(
		sqrt(pow((HEIGHTMAP_X * RAW_WIDTH * 0.5f),2) + pow((HEIGHTMAP_Z * RAW_HEIGHT * 0.5f),2)));

	//Dont add them to the root though!
	raining = false;
	snowing = false;

	return true;
}

void Renderer::DeleteParticles(){
	delete particleShader;

	//If the root isnt holding a reference, we will have to clean these
	//up!
	if (!raining) delete rainNode;
	if (!snowing) delete snowNode;
}

//Update our particles
void Renderer::UpdateParticles(float msec){
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_R)){
		raining = !raining;

		if (!raining) root->RemoveChild(rainNode);
		else root->AddChild(rainNode);
	};

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_F)){
		snowing = !snowing;

		if (!snowing) root->RemoveChild(snowNode);
		else root->AddChild(snowNode);
	};

};