#include "Renderer.h"

//This method will initialise all things debugging
bool Renderer::InitDebug(){
	basicFont = new Font(SOIL_load_OGL_texture(TEXTUREDIR"tahoma.tga", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT), 16, 16);



	sphere = new OBJMesh(MESHDIR"Sphere.obj");
	box = new OBJMesh(MESHDIR"centeredcube.obj");
	debugQuad = Mesh::GenerateQuad();

	if (!basicFont->texture)
		return false;

	gt = GameTimer();
	frames = 0;
	fps = 0;
	timeAcc = 0;

	objectsDrawn = 0;
	objectsShadowed = 0;
	drawBound = 0;

	drawWorld = true;
	physicsDrawing = false;
	octTree = false;
	broadPhase = false;
	narrowPhase = false;

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

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NUMPAD8))
		drawBound = !drawBound;

	//if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NUMPADP))

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NUMPAD7))
		drawWorld = !drawWorld;
	
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NUMPAD1))
		octTree = !octTree;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NUMPAD2))
		broadPhase = !broadPhase;
	
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NUMPAD3))
		narrowPhase = !narrowPhase;
	
	physicsDrawing = (broadPhase || narrowPhase || octTree);

}

//Method to draw the overlay
void Renderer::DrawDebugOverlay(){
	SetCurrentShader(passThrough);

	std::ostringstream buff;
	buff << "FPS: " << fps <<std::endl;
	buff << "Drawn: " << objectsDrawn << std::endl;
	buff << "Shadowed: " << objectsShadowed << std::endl;
	if (drawBound) buff << "Bounds Drawn. " << std::endl;

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
	buff << "Collisions: " << PhysicsSystem::GetPhysicsSystem().GetCollisionCount();
	if (physicsDrawing) buff << "Physics Draw: " << std::endl;
	if (octTree) buff << "OctTree," << std::endl;
	if (broadPhase) buff << "Broad phase," << std::endl;
	if (narrowPhase) buff << "Narrow phase." << std::endl;
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

void Renderer::DrawPhysics(){
	if (physicsDrawing){

		std::lock_guard<std::mutex> lock(PhysicsSystem::instance->nodesMutex);
		if (octTree) DrawOctTree();
		if (broadPhase)	DrawBroadPhase();
		

		if (narrowPhase) DrawNarrowPhase();
	}
}

void Renderer::DrawOctTree(){

	//Obtain a reference to the octree of the physics system
	OctTree& ot = PhysicsSystem::instance->octTree;

	DrawOctNode(ot.root);
}

void Renderer::DrawOctNode(const OctNode& on){
	if (on.octNodes.size() != 0){
		for (auto itr = on.octNodes.begin(); itr != on.octNodes.end(); ++itr){
			DrawOctNode(**itr);
		}
	} else {
		modelMatrix = Matrix4::Translation(on.pos) * Matrix4::Scale(Vector3(on.halfSize, on.halfSize, on.halfSize));
		//modelMatrix = Matrix4::Translation(Vector3(0,0,0)) * Matrix4::Scale(Vector3(10,10,10));;
		projMatrix = cameraProjMat;
		UpdateShaderMatrices();

		glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
			"colour"), 1, (float*) &Vector4(0,1,1,1));
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
			"useTex"), 0);

		box->Draw();
	}
}

void Renderer::DrawBroadPhase(){

	for (auto itr = PhysicsSystem::instance->allNodes.begin();
		itr != PhysicsSystem::instance->allNodes.end();
		++itr){

			if ( (*itr)->GetBroadPhaseVolume()){
				CollisionVolumeType cvt = (*itr)->GetBroadPhaseVolume()->GetType();

				switch (cvt){
				case COLLISION_PLANE:
					DrawPlane( *(Plane*) (*itr)->GetBroadPhaseVolume(), (*itr)->GetOrientation());
					break;
				case COLLISION_SPHERE:
					DrawSphere( *(CollisionSphere*) (*itr)->GetBroadPhaseVolume());
					break;
				case COLLISION_AABB:
					DrawAABB( *(CollisionAABB*) (*itr)->GetBroadPhaseVolume() );
					break;
				}
			}
	}
}

void Renderer::DrawNarrowPhase(){

	for (auto itr = PhysicsSystem::instance->allNodes.begin();
		itr != PhysicsSystem::instance->allNodes.end();
		++itr){

			if ( (*itr)->GetNarrowPhaseVolume() ){
				CollisionVolumeType cvt = (*itr)->GetNarrowPhaseVolume()->GetType();

				switch (cvt){
				case COLLISION_PLANE:
					DrawPlane( *(Plane*) (*itr)->GetBroadPhaseVolume(), (*itr)->GetOrientation());
					break;
				case COLLISION_SPHERE:
					DrawSphere( *(CollisionSphere*) (*itr)->GetBroadPhaseVolume(), &(*itr)->GetOrientation());
					break;
				case COLLISION_AABB:
					DrawAABB( *(CollisionAABB*) (*itr)->GetBroadPhaseVolume() );
					break;
				}
			}

	}

}

void Renderer::DrawSphere(const CollisionSphere& cs, const Quaternion* orientation){
	if (orientation)
		modelMatrix = Matrix4::Translation(cs.m_pos) * 
		orientation->ToMatrix() * 
		Matrix4::Scale(Vector3(cs.m_radius, cs.m_radius, cs.m_radius));
	else 
		modelMatrix = Matrix4::Translation(cs.m_pos) *
		Matrix4::Scale(Vector3(cs.m_radius, cs.m_radius, cs.m_radius));

	UpdateShaderMatrices();

	sphere->Draw();
}

void Renderer::DrawPlane(const Plane& p, const Quaternion& o){
	modelMatrix = Matrix4::Translation(p.GetNormal() * p.GetDistance()) *
		o.ToMatrix() *
		Matrix4::Scale(Vector3(1000,1000,1000));
	UpdateShaderMatrices();

	debugQuad->Draw();
}

void Renderer::DrawAABB(const CollisionAABB& aabb){

	modelMatrix = Matrix4::Translation(aabb.m_position) 
		* Matrix4::Scale(aabb.m_halfSize);
	UpdateShaderMatrices();

	box->Draw();
}

