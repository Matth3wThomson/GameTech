#include "Renderer.h"

//This method will initialise all things debugging
bool Renderer::InitDebug(){
	basicFont = new Font(SOIL_load_OGL_texture(TEXTUREDIR"tahoma.tga", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT), 16, 16);

	sphere = new OBJMesh("Sphere.obj");

	if (!basicFont->texture)
		return false;

	gt = GameTimer();
	frames = 0;
	fps = 0;
	timeAcc = 0;

	objectsDrawn = 0;
	objectsShadowed = 0;
	drawBound = 1;

	return true;
}

void Renderer::DeleteDebug(){
	delete basicFont;
}

//Count our frame rate!
void Renderer::UpdateDebug(){
	timeAcc += gt.GetTimedMS();
	frames++;
	objectsDrawn = 0;
	objectsShadowed = 0;

	//After every second update the fps variable
	if (timeAcc > 1000){
		timeAcc -= 1000;
		fps = frames;
		frames = 0;
	}
}

//Method to draw the overlay
void Renderer::DrawDebugOverlay(){
	SetCurrentShader(passThrough);

	std::ostringstream buff;
	buff << "FPS: " << fps <<std::endl;
	buff << "Drawn: " << objectsDrawn << std::endl;
	buff << "Shadowed: " << objectsShadowed << std::endl;

	DrawString(buff.str(), Vector3(0,0,0.5f), FONT_SIZE);

	buff = std::ostringstream();
	if (sobel) buff << "Sobel colour ";
	if (sobelDepth) buff << "Sobel depth ";
	if (quantizeCol) buff << "Colour quantize ";
	if (fog) buff << "Fog ";
	if (bloom) buff << "Bloom ";
	if (dubVis) buff << "Double Vision ";
	if (blur) buff << "Blur ";
	DrawString(buff.str(), Vector3(0,2*FONT_SIZE,0.5f), FONT_SIZE);
	
	buff = std::ostringstream();

	if (pause) buff << "Time paused ";
	if (dayTimeSpeedIncrease) buff << "Day/night cycle speed increased ";
	if (timeSlowed) buff << "Time slowed ";
	if (raining) buff << "Rain ";
	if (snowing) buff << "Snow ";
	DrawString(buff.str(), Vector3(0,3*FONT_SIZE,0.5f), FONT_SIZE);

	buff = std::ostringstream();
	buff << "PUPS: " << PhysicsSystem::GetPhysicsSystem().GetUpdateRate();
	buff << " No. Cols: " << PhysicsSystem::GetPhysicsSystem().GetCollisionCount();
	DrawString(buff.str(), Vector3(0, 4*FONT_SIZE, 0.5f), FONT_SIZE);

	glUseProgram(0);
}

//Method to draw the bounds of a given sceneNode
void Renderer::DrawBounds(SceneNode* n){
	
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	SetCurrentShader(passThrough);

	UpdateShaderMatrices();

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"modelMatrix"),	1,false, 
		(float*) &(Matrix4::Translation(n->GetWorldTransform().GetPositionVector()) 
		* Matrix4::Scale(
		Vector3(n->GetBoundingRadius(),
		n->GetBoundingRadius(),
		n->GetBoundingRadius()))));

	sphere->Draw();

	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
}