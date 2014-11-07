#include "Renderer.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{
	triangle = Mesh::GenerateTriangle();
	quad = Mesh::GenerateQuad();

	triangle->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"/brick.tga",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	quad->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"/chessboard.tga",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	//THIS IS GOING TO BREAK THE PROGRAM!?
	if (!triangle->GetTexture() || !quad->GetTexture()){
		return;
	}
	
	/*positions[0] = Vector3(0,0,-5);
	positions[1] = Vector3(0,0,-5);*/

	glBindTexture(GL_TEXTURE_2D, triangle->GetTexture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, quad->GetTexture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	currentShader = new Shader(SHADERDIR"TexturedVertex.glsl",
		SHADERDIR"colourFragment.glsl");

	if(!currentShader->LinkProgram()) {
		return;
	}

	usingDepth = false;
	usingAlpha = false;
	modifyObject = true;

	usingScissor = false;
	usingStencil = false;

	blendMode = 0;

	camera = new Camera();
	fov = 45.0f;

	repeating = false;
	filtering = true;

	/*projMatrix = Matrix4::Orthographic(-1,1,1,-1,1,-1);*/

	SwitchToPerspective();

	init = true;

	/*SwitchToOrthographic();*/
}
Renderer::~Renderer(void)	{
	delete triangle;
	delete quad;
	delete camera;
}

void Renderer::SwitchToPerspective(){
	projMatrix = Matrix4::Perspective(1.0f, 100000.0f,
		(float) width/ (float) height, fov);
}

void Renderer::SwitchToOrthographic(){
	projMatrix = Matrix4::Orthographic(-1.0f, 100000.0f,
		width / 2.0f, -width / 2.0f, height / 2.0f, -height / 2.0f);
}

void Renderer::UpdateScene(float msec){
	camera->UpdateCamera(msec);
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene(){
	glClearColor(0.2f,0.2f,0.2f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		| GL_STENCIL_BUFFER_BIT);	

	if (usingScissor){
		glEnable(GL_SCISSOR_TEST);
		glScissor((float) width / 2.5f, (float)height / 2.5f,
			(float) width / 5.0f, (float) height / 5.0f);
	}

	glUseProgram(currentShader->GetProgram());

	UpdateShaderMatrices();
	/*glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"textureMatrix"), 1, false, (float*)&textureMatrix);

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"viewMatrix"), 1, false, (float*)&viewMatrix);

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(),
		"projMatrix"), 1, false, (float*)&projMatrix);*/

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	if (usingStencil){
		glEnable(GL_STENCIL_TEST);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glStencilFunc(GL_ALWAYS, 2, ~0);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

		quad->Draw();

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilFunc(GL_EQUAL, 2, ~0);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	}

	glActiveTexture(GL_TEXTURE0);

	triangle->Draw();


	//Un bind our shader
	glUseProgram(0);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);

	SwapBuffers();	
}

void Renderer::UpdateTextureMatrix(float value){
	//In order to have our rotation be about the center of the texture, we 
	//must translate there, rotate once there and translate back. Hence
	//our push and pop matrices.
	Matrix4 pushPos = Matrix4::Translation(Vector3(0.5f, 0.5f, 0));
	Matrix4 popPos = Matrix4::Translation(Vector3(-0.5f, -0.5f, 0));
	Matrix4 rotation = Matrix4::Rotation(value, Vector3(0,0,1));
	textureMatrix = pushPos * rotation * popPos;
}

void Renderer::ToggleRepeating(){
	//Toggle repeating
	repeating = !repeating;

	//Rebind the triangles texture
	glBindTexture(GL_TEXTURE_2D, triangle->GetTexture());

	//Cheeky ternary if depending on the state of the repeating variable,
	//switching between repeating and clamping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, //x axis
			repeating ? GL_REPEAT : GL_CLAMP);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, //y axis
		repeating ? GL_REPEAT : GL_CLAMP);

	//Unbind to prevent unwanted changes elsewhere to our texture
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::ToggleFiltering(){
	//Toggle filtering
	filtering = !filtering;

	//Bind our texture 
	glBindTexture(GL_TEXTURE_2D, triangle->GetTexture());

	float aniso;

	if (glewIsExtensionSupported("GL_EXT_texture_filter_anisotropic"))
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);

	//Changed from GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		filtering ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		filtering ? GL_LINEAR : GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::ToggleObject(){
	modifyObject = !modifyObject;
}

void Renderer::MoveObject(float by){
	/*positions[(int)modifyObject].z += by;*/
}

void Renderer::ToggleDepth(){
	usingDepth = !usingDepth;
	usingDepth ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void Renderer::ToggleAlphaBlend(){
	usingAlpha = !usingAlpha;
	usingAlpha ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
}

void Renderer::ToggleBlendMode(){
	blendMode = (blendMode+1) % 4;

	switch(blendMode){
	case(0):glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
	case(1):glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);break;
	case(2):glBlendFunc(GL_ONE, GL_ZERO); break;
	case(3):glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;
	}
}

void Renderer::ToggleScissor(){
	usingScissor = !usingScissor;
}

void Renderer::ToggleStencil(){
	usingStencil = !usingStencil;
}