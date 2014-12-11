#include "MyGame.h"

/*
Creates a really simple scene for our game - A cube robot standing on
a floor. As the module progresses you'll see how to get the robot moving
around in a physically accurate manner, and how to stop it falling
through the floor as gravity is added to the scene. 

You can completely change all of this if you want, it's your game!

*/
MyGame::MyGame()	{
	gameCamera = new Camera(-30.0f,0.0f,Vector3(0,450,850));

	Renderer::GetRenderer().SetCamera(gameCamera);

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
	sphere	= new OBJMesh(MESHDIR"ico.obj");
	debugTex = SOIL_load_OGL_texture(TEXTUREDIR"debug.png", 
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//quad->SetTexture(debugTex);

	projectileSize = 100;
	projectileSpeed = 1;

	//allEntities.push_back(BuildRobotEntity());

	Matrix3 floorMat = Matrix3();
	floorMat.ToZero();

	GameEntity* floor = BuildQuadEntity(4000.0f, Vector3(1,0,0), 90.0f);
	floor->GetPhysicsNode().SetPosition(Vector3(0,0.0f, 0));
	floor->GetPhysicsNode().SetFixed(true);
	floor->GetPhysicsNode().SetNarrowPhaseVolume(new Plane(Vector3(0,1,0), 0.0f));

	floor->GetPhysicsNode().SetScale(Vector3(4000,4000,4000));
	floor->GetPhysicsNode().SetInverseInertiaMat(floorMat);

	floor->ConnectToSystems();
	allEntities.push_back(floor);

	GameEntity* roof = BuildQuadEntity(1000.0f, Vector3(1,0,0), 270.0f);
	roof->GetPhysicsNode().SetPosition(Vector3(0, 4000.0f, 0));
	roof->GetPhysicsNode().SetFixed(true);
	roof->GetPhysicsNode().SetNarrowPhaseVolume(new Plane(Vector3(0,-1,0), -4000.0f));

	roof->GetPhysicsNode().SetScale(Vector3(4000,4000,4000));
	roof->GetPhysicsNode().SetInverseInertiaMat(floorMat);

	roof->ConnectToSystems();
	allEntities.push_back(roof);

	GameEntity* wallBack = BuildQuadEntity(4000.0f, Vector3(1,0,0), 0.0f);
	wallBack->GetPhysicsNode().SetPosition(Vector3(0,0,4000.0f));
	wallBack->GetPhysicsNode().SetFixed(true);
	wallBack->GetPhysicsNode().SetScale(Vector3(4000,4000,4000));

	wallBack->GetPhysicsNode().SetNarrowPhaseVolume(new Plane(Vector3(0,0,-1), -4000.0f));
	
	wallBack->GetPhysicsNode().SetInverseInertiaMat(floorMat);

	wallBack->ConnectToSystems();
	allEntities.push_back(wallBack);

	GameEntity* wallFront = BuildQuadEntity(4000.0f, Vector3(1,0,0), 180.0f);
	wallFront->GetPhysicsNode().SetPosition(Vector3(0,0,-4000.0f));
	wallFront->GetPhysicsNode().SetFixed(true);
	wallFront->GetPhysicsNode().SetScale(Vector3(4000,4000,4000));

	wallFront->GetPhysicsNode().SetNarrowPhaseVolume(new Plane(Vector3(0,0,1), -4000.0f));

	wallFront->GetPhysicsNode().SetInverseInertiaMat(floorMat);
	wallFront->ConnectToSystems();
	allEntities.push_back(wallFront);

	GameEntity* wallLeft = BuildQuadEntity(4000.0f, Vector3(0,1,0), 270.0f);
	wallLeft->GetPhysicsNode().SetPosition(Vector3(-4000.0f,0,0));
	wallLeft->GetPhysicsNode().SetFixed(true);
	wallLeft->GetPhysicsNode().SetScale(Vector3(4000,4000,4000));

	wallLeft->GetPhysicsNode().SetNarrowPhaseVolume(new Plane(Vector3(1,0,0), -4000.0f));

	wallLeft->GetPhysicsNode().SetInverseInertiaMat(floorMat);
	wallLeft->ConnectToSystems();
	allEntities.push_back(wallLeft);

	GameEntity* wallRight = BuildQuadEntity(4000.0f, Vector3(0,1,0), 90.0f);
	wallRight->GetPhysicsNode().SetPosition(Vector3(4000,0,0));
	wallRight->GetPhysicsNode().SetFixed(true);
	wallRight->GetPhysicsNode().SetScale(Vector3(4000,4000,4000));

	wallRight->GetPhysicsNode().SetNarrowPhaseVolume(new Plane(Vector3(-1,0,0), -4000.0f));

	wallRight->GetPhysicsNode().SetInverseInertiaMat(floorMat);
	wallRight->ConnectToSystems();
	allEntities.push_back(wallRight);

	GameEntity* sphere1 = BuildSphereEntity(projectileSize);
	sphere1->GetPhysicsNode().SetPosition(Vector3(0,1000,0));
	sphere1->GetPhysicsNode().SetFixed(true);
	/*if (Window::GetKeyboard()->KeyDown(KEYBOARD_C))*/
	
	//sphere1->GetPhysicsNode().ApplyForce(Vector3(0,-0.1f,0)); //Add some gravity
	//sphere1->GetPhysicsNode().SetConstantAccel(Vector3(0, -0.01f, 0));
	sphere1->GetPhysicsNode().SetMass(100.0f);
	//sphere1->GetPhysicsNode().SetOrientation(Quaternion(RAND(), RAND(), RAND(), RAND()));
	sphere1->GetPhysicsNode().SetInvSphereInertiaMatrix(100, projectileSize);
	sphere1->GetPhysicsNode().SetNarrowPhaseVolume(new CollisionSphere(Vector3(0,0,0), projectileSize));
	sphere1->GetPhysicsNode().SetScale(Vector3(projectileSize, projectileSize, projectileSize));

	sphere1->ConnectToSystems();

	allEntities.push_back(sphere1);

	GameEntity* sphere2 = BuildSphereEntity(projectileSize);
	sphere2->GetPhysicsNode().SetPosition(Vector3(0,500,0));
	/*if (Window::GetKeyboard()->KeyDown(KEYBOARD_C))*/
	
	//sphere2->GetPhysicsNode().ApplyForce(Vector3(0,-0.1f,0)); //Add some gravity
	sphere2->GetPhysicsNode().SetConstantAccel(Vector3(0, -0.01f, 0));
	sphere2->GetPhysicsNode().SetMass(100.0f);
	//sphere2->GetPhysicsNode().SetOrientation(Quaternion(RAND(), RAND(), RAND(), RAND()));
	sphere2->GetPhysicsNode().SetInvSphereInertiaMatrix(100, projectileSize);
	sphere2->GetPhysicsNode().SetNarrowPhaseVolume(new CollisionSphere(Vector3(0,0,0), projectileSize));
	sphere2->GetPhysicsNode().SetScale(Vector3(projectileSize, projectileSize, projectileSize));

	sphere2->ConnectToSystems();

	allEntities.push_back(sphere2);

	TreeEntity* te = new TreeEntity(750.0f);
	te->GetPhysicsNode().SetPosition(Vector3(700, 35, 800));
	te->ConnectToSystems();
	allEntities.push_back(te);

	GameEntity* hm = BuildHeightmapEntity();
	hm->ConnectToSystems();
	allEntities.push_back(hm);

	Spring* s = new Spring(&sphere1->GetPhysicsNode(), Vector3(0,0,0),
		&sphere2->GetPhysicsNode(), Vector3(0, -500, 0));

	PhysicsSystem::GetPhysicsSystem().AddConstraint(s);
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
		projectileSpeed--;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_K))
		projectileSize++;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
		projectileSize--;
	
	//Spawn a new object.
	if (Window::GetMouse()->ButtonTriggered(MOUSE_LEFT)){

		GameEntity* ge = BuildCubeEntity(projectileSize);
		ge->GetPhysicsNode().SetPosition(gameCamera->GetPosition() + gameCamera->GetDirectionVector() * 100.0f);
		ge->GetPhysicsNode().SetLinearVelocity(gameCamera->GetDirectionVector() * projectileSpeed);

		//ge->GetPhysicsNode().ApplyForce(Vector3(0,-0.1f, 0)); //Add Gravity
		ge->GetPhysicsNode().SetConstantAccel(Vector3(0,-0.01f, 0));
		ge->GetPhysicsNode().SetMass(100.0f);
		ge->GetPhysicsNode().SetScale(Vector3(projectileSize, projectileSize, projectileSize));

		ge->GetPhysicsNode().SetNarrowPhaseVolume(new CollisionConvex(centCube));
		ge->GetPhysicsNode().SetBroadPhaseVolume(new CollisionAABB(
			ge->GetPhysicsNode().GetPosition(), ge->GetPhysicsNode().GetScale() * 10.0f));

		ge->GetPhysicsNode().SetOrientation(Quaternion(RAND(), RAND(), RAND(), RAND()).Normalise());
		ge->GetPhysicsNode().SetInvCuboidInertiaMatrix(100, projectileSize, projectileSize, projectileSize);
		
		ge->ConnectToSystems();

		allEntities.push_back(ge);
	}

	if (Window::GetMouse()->ButtonTriggered(MOUSE_RIGHT)){

		GameEntity* ge = BuildSphereEntity(projectileSize);
		ge->GetPhysicsNode().SetPosition(gameCamera->GetPosition() + gameCamera->GetDirectionVector() * 100.0f);
		/*if (Window::GetKeyboard()->KeyDown(KEYBOARD_C))*/
		ge->GetPhysicsNode().SetLinearVelocity(gameCamera->GetDirectionVector() * projectileSpeed);
		//ge->GetPhysicsNode().ApplyForce(Vector3(0,-0.1f,0)); //Add some gravity
		ge->GetPhysicsNode().SetConstantAccel(Vector3(0, -0.01f, 0));
		ge->GetPhysicsNode().SetMass(100.0f);
		//ge->GetPhysicsNode().SetOrientation(Quaternion(RAND(), RAND(), RAND(), RAND()));
		ge->GetPhysicsNode().SetInvSphereInertiaMatrix(100, projectileSize);
		ge->GetPhysicsNode().SetScale(Vector3(projectileSize, projectileSize, projectileSize));

		ge->GetPhysicsNode().SetNarrowPhaseVolume(new CollisionSphere(Vector3(0,0,0), projectileSize));

		ge->ConnectToSystems();

		allEntities.push_back(ge);
	}

	for(vector<GameEntity*>::iterator i = allEntities.begin(); i != allEntities.end(); ++i) {
		(*i)->Update(msec);
	}

	/*
	Here's how we can use OGLRenderer's inbuilt debug-drawing functions! 
	I meant to talk about these in the graphics module - Oops!

	We can draw squares, lines, crosses and circles, of varying size and
	colour - in either perspective or orthographic mode.

	Orthographic debug drawing uses a 'virtual canvas' of 720 * 480 - 
	that is 0,0 is the top left, and 720,480 is the bottom right. A function
	inside OGLRenderer is provided to convert clip space coordinates into
	this canvas space coordinates. How you determine clip space is up to you -
	maybe your renderer has getters for the view and projection matrix?

	Or maybe your Camera class could be extended to contain a projection matrix?
	Then your game would be able to determine clip space coordinates for its
	active Camera without having to involve the Renderer at all?

	Perspective debug drawing relies on the view and projection matrices inside
	the renderer being correct at the point where 'SwapBuffers' is called. As
	long as these are valid, your perspective drawing will appear in the world.

	This gets a bit more tricky with advanced rendering techniques like deferred
	rendering, as there's no guarantee of the state of the depth buffer, or that
	the perspective matrix isn't orthographic. Therefore, you might want to draw
	your debug lines before the inbuilt position before SwapBuffers - there are
	two OGLRenderer functions DrawDebugPerspective and DrawDebugOrtho that can
	be called at the appropriate place in the pipeline. Both take in a viewProj
	matrix as an optional parameter.

	Debug rendering uses its own debug shader, and so should be unaffected by
	and shader changes made 'outside' of debug drawing

	*/
	//Lets draw a box around the cube robot!
	//Renderer::GetRenderer().DrawDebugBox(DEBUGDRAW_PERSPECTIVE, Vector3(0,51,0), Vector3(100,100,100), Vector3(1,0,0));

	////We'll assume he's aiming at something...so let's draw a line from the cube robot to the target
	////The 1 on the y axis is simply to prevent z-fighting!
	//Renderer::GetRenderer().DrawDebugLine(DEBUGDRAW_PERSPECTIVE, Vector3(0,1,0),Vector3(200,1,200), Vector3(0,0,1), Vector3(1,0,0));

	////Maybe he's looking for treasure? X marks the spot!
	//Renderer::GetRenderer().DrawDebugCross(DEBUGDRAW_PERSPECTIVE, Vector3(200,1,200),Vector3(50,50,50), Vector3(0,0,0));

	////CubeRobot is looking at his treasure map upside down!, the treasure's really here...
	//Renderer::GetRenderer().DrawDebugCircle(DEBUGDRAW_PERSPECTIVE, Vector3(-200,1,-200),50.0f, Vector3(0,1,0));
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

GameEntity* MyGame::BuildHeightmapEntity(){
	heightMap = new HeightMap(TEXTUREDIR"terrain.raw");

	heightMap->SetTexture(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren reds.jpg", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	heightMap->SetBumpMap(SOIL_load_OGL_texture(
		TEXTUREDIR"Barren redsDOT3.jpg", SOIL_LOAD_AUTO, 
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	glBindTexture(GL_TEXTURE_2D, heightMap->GetTexture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	GameEntity* ge = new GameEntity(new SceneNode(heightMap), NULL);
	ge->GetRenderNode().SetBoundingRadius( 
		sqrt( pow(RAW_WIDTH * HEIGHTMAP_X * 0.5f, 3)
		+ pow(RAW_HEIGHT * HEIGHTMAP_Z * 0.5f, 2)));
	return ge;
}