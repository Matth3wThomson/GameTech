#include "Renderer.h"

bool Renderer::InitDebug(){
	basicFont = new Font(SOIL_load_OGL_texture(TEXTUREDIR"tahoma.tga", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT), 16, 16);

	if (!basicFont->texture)
		return false;

	frames = 0;
	fps = 0;
	timeAcc = 0;

	objectsDrawn = 0;
	objectsShadowed = 0;
	drawBound = false;

	return true;
}

void Renderer::DeleteDebug(){

	delete basicFont;
}

void Renderer::UpdateDebug(float msec){
	timeAcc += msec;
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

void Renderer::DrawDebugOverlay(){
	SetCurrentShader(passThrough);

	std::ostringstream buff;
	buff << "FPS: " << fps <<std::endl;
	buff << "Drawn: " << objectsDrawn << std::endl;
	buff << "Shadowed: " << objectsShadowed << std::endl;

	DrawString(buff.str(), Vector3(0,0,0.5f), 16.0f);

	buff = std::ostringstream();
	if (sobel) buff << "Sobel colour ";
	if (sobelDepth) buff << "Sobel depth ";
	if (antiAlias) buff << "Sobel AA ";
	if (bloom) buff << "Bloom ";
	if (dubVis) buff << "Double Vision ";
	if (blur) buff << "Blur ";

	DrawString(buff.str(), Vector3(0,16.0f,0.5f), 16.0f);
	

	glUseProgram(0);
}

//TODO: This method assumes that the model matrix is already set accordingly... correct?
void Renderer::DrawBounds(SceneNode* n){
	
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	SetCurrentShader(passThrough); //NEW

	UpdateShaderMatrices(); //NEW

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"modelMatrix"),	1,false, 
		(float*) &(Matrix4::Translation(n->GetWorldTransform().GetPositionVector()) 
		* Matrix4::Scale(
		Vector3(n->GetBoundingRadius(),
		n->GetBoundingRadius(),
		n->GetBoundingRadius()))));

	sphere->Draw();

	glUseProgram(0); //NEW
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}



//TODO: Lots of reinstantiation of view and proj matrix!
void Renderer::DrawFrustum(){
	Plane* planes = frameFrustum.Get6Planes();

	SetCurrentShader(passThrough);

	for (int i=0; i<6; ++i){
		modelMatrix = Matrix4::Translation(planes[i].GetNormal()
			* planes[i].GetDistance())
			* Matrix4::Translation(light->GetPosition())
			* Matrix4::Scale(Vector3(1000,1000,1))
			* Matrix4::Rotation(90, -planes[i].GetNormal());

		viewMatrix = camera->BuildViewMatrix();
		projMatrix = cameraProjMat;

		UpdateShaderMatrices();

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		quad->SetTexture(0);
		quad->Draw();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	}

	glUseProgram(0);
}