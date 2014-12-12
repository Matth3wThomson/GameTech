#include "MyGame.h"

/*
Creates a really simple scene for our game - A cube robot standing on
a floor. As the module progresses you'll see how to get the robot moving
around in a physically accurate manner, and how to stop it falling
through the floor as gravity is added to the scene. 

You can completely change all of this if you want, it's your game!

*/
MyGame::MyGame(){

	gameCamera = new Camera(-30.0f,0.0f,Vector3(0,450,850));

	Renderer::GetRenderer().SetCamera(gameCamera);
	gameCamera->SetPosition(Vector3(0, 1500, 0));
	gameCamera->SetPitch(-90);

	CubeRobot::CreateCube();

	/*
	We're going to manage the meshes we need in our game in the game class!

	You can do this with textures, too if you want - but you might want to make
	some sort of 'get / load texture' part of the Renderer or OGLRenderer, so as
	to encapsulate the API-specific parts of texture loading behind a class so
	we don't care whether the renderer is OpenGL / Direct3D / using SOIL or 
	something else...
	*/
	cube	= new OBJMesh(MESHDIR"cube.obj");
	centCube = new OBJMesh(MESHDIR"centeredcube.obj");
	quad	= Mesh::GenerateQuad();
	cylinder = Mesh::GenerateCylinder(20);
	sphere	= new OBJMesh(MESHDIR"ico.obj");
	debugTex = SOIL_load_OGL_texture(TEXTUREDIR"debug.png", 
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	quad->SetTexture(debugTex);

	projectileSize = 100;
	projectileSpeed = 1;

	Matrix3 floorMat = Matrix3();
	floorMat.ToZero();

	GameEntity* floor = new GameEntity(NULL, new PhysicsNode());
	floor->GetPhysicsNode().SetPosition(Vector3(0,0.0f, 0));
	floor->GetPhysicsNode().SetFixed(true);
	floor->GetPhysicsNode().SetNarrowPhaseVolume(new Plane(Vector3(0,1,0), 0.0f));

	floor->GetPhysicsNode().SetScale(Vector3(4000,4000,4000));
	floor->GetPhysicsNode().SetInverseInertiaMat(floorMat);

	/*floor->GetRenderNode().SetShader(Renderer::GetRenderer().sceneShader);
	floor->GetRenderNode().SetShaderUpdateFunc([]{ Renderer::GetRenderer().UpdateCombineSceneShaderMatricesPO(); } );*/

	floor->ConnectToSystems();
	allEntities.push_back(floor);

	GameEntity* roof = new GameEntity(NULL, new PhysicsNode());
	roof->GetPhysicsNode().SetPosition(Vector3(0, 8000.0f, 0));
	roof->GetPhysicsNode().SetFixed(true);
	roof->GetPhysicsNode().SetNarrowPhaseVolume(new Plane(Vector3(0,-1,0), -8000.0f));

	roof->GetPhysicsNode().SetScale(Vector3(4000,4000,4000));
	roof->GetPhysicsNode().SetInverseInertiaMat(floorMat);

	roof->ConnectToSystems();
	allEntities.push_back(roof);

	GameEntity* wallBack = new GameEntity(NULL, new PhysicsNode());
	wallBack->GetPhysicsNode().SetPosition(Vector3(0,0,4000.0f));
	wallBack->GetPhysicsNode().SetFixed(true);
	wallBack->GetPhysicsNode().SetScale(Vector3(4000,4000,4000));

	wallBack->GetPhysicsNode().SetNarrowPhaseVolume(new Plane(Vector3(0,0,-1), -4000.0f));
	
	wallBack->GetPhysicsNode().SetInverseInertiaMat(floorMat);

	wallBack->ConnectToSystems();
	allEntities.push_back(wallBack);

	GameEntity* wallFront = new GameEntity(NULL, new PhysicsNode());
	wallFront->GetPhysicsNode().SetPosition(Vector3(0,0,-4000.0f));
	wallFront->GetPhysicsNode().SetFixed(true);
	wallFront->GetPhysicsNode().SetScale(Vector3(4000,4000,4000));

	wallFront->GetPhysicsNode().SetNarrowPhaseVolume(new Plane(Vector3(0,0,1), -4000.0f));

	wallFront->GetPhysicsNode().SetInverseInertiaMat(floorMat);
	wallFront->ConnectToSystems();
	allEntities.push_back(wallFront);

	GameEntity* wallLeft = new GameEntity(NULL, new PhysicsNode());
	wallLeft->GetPhysicsNode().SetPosition(Vector3(-4000.0f,0,0));
	wallLeft->GetPhysicsNode().SetFixed(true);
	wallLeft->GetPhysicsNode().SetScale(Vector3(4000,4000,4000));

	wallLeft->GetPhysicsNode().SetNarrowPhaseVolume(new Plane(Vector3(1,0,0), -4000.0f));

	wallLeft->GetPhysicsNode().SetInverseInertiaMat(floorMat);
	wallLeft->ConnectToSystems();
	allEntities.push_back(wallLeft);

	GameEntity* wallRight = new GameEntity(NULL, new PhysicsNode());
	wallRight->GetPhysicsNode().SetPosition(Vector3(4000,0,0));
	wallRight->GetPhysicsNode().SetFixed(true);
	wallRight->GetPhysicsNode().SetScale(Vector3(4000,4000,4000));

	wallRight->GetPhysicsNode().SetNarrowPhaseVolume(new Plane(Vector3(-1,0,0), -4000.0f));

	wallRight->GetPhysicsNode().SetInverseInertiaMat(floorMat);
	wallRight->ConnectToSystems();
	allEntities.push_back(wallRight);

	Cloth * c = new Cloth(5, 8, Vector3(1900,1500,0), Vector3(600,500,1000), 120.0f, TEXTUREDIR"unionjack.png");
	c->ConnectToSystems();
	allEntities.push_back(c);

	TreeEntity* te = new TreeEntity(750.0f);
	te->GetPhysicsNode().SetPosition(Vector3(700, 35, 800));

	te->GetRenderNode().SetShader(Renderer::GetRenderer().sceneShader);
	te->GetRenderNode().SetShaderUpdateFunc([]{ Renderer::GetRenderer().UpdateCombineSceneShaderMatricesPO(); } );

	te->ConnectToSystems();
	allEntities.push_back(te);

	//Randomly creates half respawnable entities and half destructable entities
	for (int i=0; i<BALLS_X; ++i){
		for (int j=0; j<BALLS_Z; ++j){
			if (rand() % 2 == 0){

			RespawnableEntity* ball = BuildRespawnableSphere(100.0f, 
				Vector3(((i/BALLS_X) - 0.5f)
				* WORLD_SIZE, WORLD_SIZE,
				((j/BALLS_Z) - 0.5f) * WORLD_SIZE), 1000);
			PhysicsNode& pn = ball->GetPhysicsNode();

			pn.SetConstantAccel(Vector3(0, -0.01f, 0));
			pn.SetPosition(Vector3( ((i/BALLS_X) - 0.5f)
				* WORLD_SIZE, WORLD_SIZE,
				((j/BALLS_Z) - 0.5f) * WORLD_SIZE));
			pn.SetBroadPhaseVolume(new CollisionSphere(pn.GetPosition(), 100.0f));
			pn.SetInvSphereInertiaMatrix(100, 100.0f);
			pn.SetMass(100.0f);
			pn.SetScale(Vector3(100, 100, 100));
			pn.Sleep();

			ball->GetRenderNode().SetShader(Renderer::GetRenderer().phong);
			ball->GetRenderNode().SetColour(Vector4(1, 0, 1, 1.0f));

			ball->ConnectToSystems();
			allEntities.push_back(ball);

			} else {
				DestructibleSphere* ds = new DestructibleSphere(100, sphere, 100,
					Vector3(((i/BALLS_X) - 0.5f)
				* WORLD_SIZE, WORLD_SIZE,
				((j/BALLS_Z) - 0.5f) * WORLD_SIZE), 5);
				ds->ConnectToSystems();
				allEntities.push_back(ds);
			}
		}
	}
}

MyGame::~MyGame(void)	{
	/*
	We're done with our assets now, so we can delete them
	*/
	delete cube;
	delete quad;
	delete sphere;

	CubeRobot::DeleteCube();

	//GameClass destructor will destroy your entities for you...
}

/*
Here's the base 'skeleton' of your game update loop! You will presumably
want your games to have some sort of internal logic to them, and this
logic will be added to this function.
*/
void MyGame::UpdateGame(float msec) {
	if(gameCamera) {
		gameCamera->UpdateCamera(msec);
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_M))
		projectileSpeed++;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_N))
		projectileSpeed = max(0, projectileSpeed-1);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_K))
		projectileSize+=10.0f;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
		projectileSize = max(0, projectileSize-10.0f);
	
	//Spawn a new object.
	if (Window::GetMouse()->ButtonTriggered(MOUSE_LEFT)){

		GameEntity* ge = BuildCubeEntity(projectileSize);
		ge->GetPhysicsNode().SetPosition(gameCamera->GetPosition() + gameCamera->GetDirectionVector() * 100.0f);
		ge->GetPhysicsNode().SetLinearVelocity(gameCamera->GetDirectionVector() * projectileSpeed);

		ge->GetPhysicsNode().SetConstantAccel(Vector3(0,-0.01f, 0));
		ge->GetPhysicsNode().SetMass(100.0f);
		ge->GetPhysicsNode().SetScale(Vector3(projectileSize, projectileSize, projectileSize));

		ge->GetPhysicsNode().SetNarrowPhaseVolume(new CollisionConvex(centCube));
		ge->GetPhysicsNode().SetBroadPhaseVolume(new CollisionAABB(
			ge->GetPhysicsNode().GetPosition(), ge->GetPhysicsNode().GetScale() * 10.0f));

		ge->GetPhysicsNode().SetOrientation(Quaternion(RAND(), RAND(), RAND(), RAND()).Normalise());
		ge->GetPhysicsNode().SetInvCuboidInertiaMatrix(100, projectileSize, projectileSize, projectileSize);
		
		ge->GetRenderNode().SetColour(Vector4(RAND(), RAND(), RAND(), 1.0f));

		ge->GetRenderNode().SetShader(Renderer::GetRenderer().phong);

		ge->ConnectToSystems();

		allEntities.push_back(ge);
	}

	if (Window::GetMouse()->ButtonTriggered(MOUSE_RIGHT)){

		GameEntity* ge = BuildSphereEntity(projectileSize);
		ge->GetPhysicsNode().SetPosition(gameCamera->GetPosition() + gameCamera->GetDirectionVector() * 100.0f);

		ge->GetPhysicsNode().SetLinearVelocity(gameCamera->GetDirectionVector() * projectileSpeed);
		
		ge->GetPhysicsNode().SetConstantAccel(Vector3(0, -0.01f, 0));
		ge->GetPhysicsNode().SetMass(100.0f);
		
		ge->GetPhysicsNode().SetInvSphereInertiaMatrix(100, projectileSize);
		ge->GetPhysicsNode().SetScale(Vector3(projectileSize, projectileSize, projectileSize));

		ge->GetPhysicsNode().SetNarrowPhaseVolume(new CollisionSphere(Vector3(0,0,0), projectileSize));

		ge->GetRenderNode().SetShader(Renderer::GetRenderer().phong);
		ge->GetRenderNode().SetColour(Vector4(RAND(), RAND(), RAND(), 1.0f));

		ge->ConnectToSystems();

		allEntities.push_back(ge);
	}

	if (Window::GetMouse()->ButtonTriggered(MOUSE_MIDDLE)){

		GameEntity* ge = new GameEntity(new SceneNode(cylinder), new PhysicsNode());
		ge->GetPhysicsNode().SetPosition(gameCamera->GetPosition() + gameCamera->GetDirectionVector() * 100.0f);
		ge->GetPhysicsNode().SetLinearVelocity(gameCamera->GetDirectionVector() * projectileSpeed);

		ge->GetPhysicsNode().SetConstantAccel(Vector3(0,-0.01f, 0));
		ge->GetPhysicsNode().SetMass(100.0f);
		ge->GetPhysicsNode().SetScale(Vector3(projectileSize * 0.1f, projectileSize, projectileSize * 0.1f));

		ge->GetPhysicsNode().SetNarrowPhaseVolume(new CollisionConvex(cylinder));
		ge->GetPhysicsNode().SetBroadPhaseVolume(new CollisionSphere(
			ge->GetPhysicsNode().GetPosition(), ge->GetPhysicsNode().GetScale().y));

		ge->GetPhysicsNode().SetOrientation(Quaternion(RAND(), RAND(), RAND(), RAND()).Normalise());
		ge->GetPhysicsNode().SetInvCuboidInertiaMatrix(100, projectileSize, projectileSize, projectileSize);
		
		ge->GetRenderNode().SetShader(Renderer::GetRenderer().phong);
		ge->GetRenderNode().SetColour(Vector4(RAND(), RAND(), RAND(), 1.0f));

		ge->ConnectToSystems();

		allEntities.push_back(ge);
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_G)){
		PhysicsSystem::GetPhysicsSystem().WakeAllNodes();
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_C)){
		msec *= 0.1f;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_V)){
		msec *= 10.0f;
	}

	std::ostringstream buff = std::ostringstream();
	buff << "Projectile size(J/K): " << projectileSize << " Projectile speed(N/M): " << projectileSpeed;

	Renderer::GetRenderer().SetOverlayText(buff.str());

	for(vector<GameEntity*>::iterator i = allEntities.begin(); i != allEntities.end(); ++i) {
		(*i)->Update(msec);
	}
}

/*
Makes an entity that looks like a CubeRobot! You'll probably want to modify
this so that you can have different sized robots, with different masses and
so on!
*/
GameEntity* MyGame::BuildRobotEntity() {
	GameEntity*g = new GameEntity(new CubeRobot(), new PhysicsNode());

	g->ConnectToSystems();
	return g;
}

/*
Makes a cube. Every game has a crate in it somewhere!
*/
GameEntity* MyGame::BuildCubeEntity(float size) {
	GameEntity*g = new GameEntity(new SceneNode(centCube), new PhysicsNode());
	//g->ConnectToSystems();

	SceneNode &test = g->GetRenderNode();

	test.SetModelScale(Vector3(size,size,size));
	test.SetBoundingRadius(size * 1.5f);

	return g;
}
/*
Makes a sphere.
*/
GameEntity* MyGame::BuildSphereEntity(float radius) {
	SceneNode* s = new SceneNode(sphere);

	s->SetModelScale(Vector3(radius,radius,radius));
	s->SetBoundingRadius(radius);

	GameEntity*g = new GameEntity(s, new PhysicsNode());
	//g->ConnectToSystems();
	return g;
}

RespawnableEntity* MyGame::BuildRespawnableSphere(float radius, const Vector3& position, float respawnTime){
	SceneNode* s = new SceneNode(sphere);

	s->SetModelScale(Vector3(radius, radius, radius));
	s->SetBoundingRadius(radius);

	RespawnableEntity* sphere = new RespawnableEntity(s, new PhysicsNode(), position, respawnTime, 150);
	return sphere;
}

/*
Makes a flat quad, initially oriented such that we can use it as a simple
floor. 
*/
GameEntity* MyGame::BuildQuadEntity(float size, const Vector3& orientation, float angle){

	SceneNode* s = new SceneNode(quad);

	s->SetModelScale(Vector3(size,size,size));
	//Oh if only we had a set texture function...we could make our brick floor again WINK WINK

	//Simple pythag
	s->SetBoundingRadius(sqrtf(size*size + size*size));

	PhysicsNode*p = new PhysicsNode(Quaternion::AxisAngleToQuaterion(orientation, angle), Vector3());

	GameEntity*g = new GameEntity(s, p);
	//g->ConnectToSystems();
	return g;
}