#include "TreeEntity.h"

int TreeEntity::instantiatedObjects = 0;

Mesh* TreeEntity::cylinder = NULL;
GLuint TreeEntity::trunkTex = 0;
GLuint TreeEntity::trunkToonTex = 0;

//Leaves
float TreeEntity::growthSpeed = 0.9f; //Higher means slower!
Mesh* TreeEntity::leafQuad = NULL;
GLuint TreeEntity::leafTex = 0;
GLuint TreeEntity::leafToonTex = 0;

//Static fruit members
Mesh* TreeEntity::fruitMesh = NULL;
Shader* TreeEntity::fruitShader = NULL;

//Particle emission
Shader* TreeEntity::particleShader = NULL;
GLuint TreeEntity::particleTex = 0;

TreeEntity::TreeEntity(const float maxTreeHeight)
{
	GPUBytes = 0;
	instantiatedObjects++;

	if (!cylinder){
		cylinder = Mesh::GenerateCylinder(16, 0.1f);

		GPUBytes += cylinder->GetGPUMemoryUsed();

		trunkTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.jpg",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

		cylinder->SetTexture(trunkTex);


		GPUBytes += ((unsigned int) (GetLastTextureGPUMem() * 1.33));

		trunkToonTex = SOIL_load_OGL_texture(TEXTUREDIR"mc wood.jpg",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

		GPUBytes += ((unsigned int) (GetLastTextureGPUMem() * 1.33));

		cylinder->SetBumpMap(SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.jpg",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

		GPUBytes += ((unsigned int) (GetLastTextureGPUMem() * 1.33));

		if (!cylinder->GetTexture() || !cylinder->GetBumpMap() || !trunkToonTex)
			return;

		//particleShader = pShader; //TODO: Sort out shading

		glBindTexture(GL_TEXTURE_2D, cylinder->GetTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindTexture(GL_TEXTURE_2D, cylinder->GetBumpMap());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	//Leaves
	if (!leafQuad){
		leafQuad = Mesh::GenerateBAQuad(true);
		GPUBytes += leafQuad->GetGPUMemoryUsed();

		leafTex = SOIL_load_OGL_texture(TEXTUREDIR"leaf6.png",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
		leafQuad->SetTexture(leafTex);
		GPUBytes += ((unsigned int) (GetLastTextureGPUMem() * 1.33));

		//Changed from green.png
		leafToonTex = SOIL_load_OGL_texture(TEXTUREDIR"darkGreen.png",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
		GPUBytes += ((unsigned int) (GetLastTextureGPUMem() * 1.33));

		leafQuad->SetBumpMap(SOIL_load_OGL_texture(TEXTUREDIR"leaf6bump.png",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
		GPUBytes += ((unsigned int) (GetLastTextureGPUMem() * 1.33));

		if (!leafQuad->GetTexture() || !leafQuad->GetBumpMap() || !leafToonTex)
			return;
	}

	//Fruit
	if (!fruitMesh){ 
		fruitMesh = new OBJMesh(MESHDIR"sphere.obj");

		GPUBytes += fruitMesh->GetGPUMemoryUsed();

		GLuint fruitTex = SOIL_load_OGL_texture(TEXTUREDIR"red.jpg",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

		GPUBytes += ((unsigned int) (GetLastTextureGPUMem() * 1.33));

		//fruitShader = fruitShad; //Sort shading out!

		if (!fruitTex) 
			return;

		fruitMesh->SetTexture(fruitTex);
	}

	//Particles
	if (!particleTex){
		
		particleTex = SOIL_load_OGL_texture(TEXTUREDIR"particle.tga",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

		GPUBytes += ((unsigned int) (GetLastTextureGPUMem() * 1.33));

	}

	this->maxTreeHeight = maxTreeHeight;
	stillGrowing = true; 

	depth = 0;
	timePassed = 0;
	branchInterval = 0.1f;
	numLeaves = 0;
	lastBranch = branchInterval;

	fruitBearing = false;
	fruitGrown = true;
	fruitStartGrowth = 0;

	//Need to instantiate a render node first!
	physicsNode = new PhysicsNode();
	physicsNode->SetBroadPhaseVolume(new CollisionAABB(Vector3(100, 100, 100), Vector3(1000, 1000, 1000)));
	physicsNode->SetNarrowPhaseVolume(new CollisionConvex(cylinder));
	physicsNode->SetFixed(true);
	renderNode = new SceneNode(cylinder);
}

TreeEntity::TreeEntity(Mesh* m, int depth, float maxBranchHeight){
	renderNode = new SceneNode(m);

	this->maxTreeHeight = maxBranchHeight;

	growthAmount = 0;
	this->depth = depth;
	stillGrowing = true;
	timePassed = 0;
	numLeaves = 0;
	branchInterval = 0.1f;
	lastBranch = branchInterval;

	fruitBearing = false;
	fruitGrown = true;
	fruitStartGrowth = 0;

}


TreeEntity::~TreeEntity(void)
{

}

void TreeEntity::Update(float msec){
	timePassed += msec/1000; //Convert to seconds (For performance on tendToOne equation)

	if (stillGrowing){
		growthAmount = TendToOne(timePassed);

		if ( (1 - growthAmount) < TOLERANCE){
			if (fruitBearing){
				//Set fruit growing...
				fruitStartGrowth = timePassed;
				//AddFruit(this);
			}
			stillGrowing = false;
		}

		//We only want the branches to grow to a smaller size than the main tree!
		if (depth != 0){
			growthAmount /= (depth+1);
		}
	}

	//Only add the particle emitter once the fruit has grown!
	if (fruitBearing){
		if (!fruitGrown){
			if ( (fruitStartGrowth + (FRUIT_GROW_TIME * 0.001)) < timePassed){
				fruitGrown = true;
				//AddParticleEmitter();
			}
		}
	}

	physicsNode->SetScale(Vector3(growthAmount*0.05f * maxTreeHeight,
			growthAmount * maxTreeHeight,
			growthAmount* 0.05f * maxTreeHeight));

	renderNode->SetBoundingRadius(growthAmount  * maxTreeHeight);

	//Dont endlessly grow the tree....
	if (depth < MAX_DEPTH){
		//Add branches if it this node is not at the branch limit
		if (growthAmount * (depth+1) > lastBranch){
			lastBranch+= branchInterval;
			AddRandomBranch();
		}
	} else if (depth == MAX_DEPTH) {
		//Else add leaves!
		if (numLeaves < MAX_LEAVES && (growthAmount *= (depth+1)) > 0.9f){
			AddRandomLeaf();
			numLeaves++;
		}
	}

	GameEntity::Update(msec);
}

float TreeEntity::TendToOne(float x){
	return -pow( growthSpeed, x )+ 1;
}

void TreeEntity::AddRandomBranch(){

	//Create another tree node with a depth value of this+1
	TreeEntity* branch = new TreeEntity(renderNode->GetMesh() , depth+1, (maxTreeHeight / (depth+2.0f)) + maxTreeHeight / 7.0f);

	branch->physicsNode = new PhysicsNode();

	Vector3 branchPos = this->physicsNode->GetPosition();

	Quaternion& parentOrientation = this->GetPhysicsNode().GetOrientation();

	parentOrientation.ToMatrix();

	branchPos = parentOrientation.ToMatrix() * (branchPos + Vector3(0,growthAmount * maxTreeHeight,0));

	//branchPos.y += growthAmount * maxTreeHeight;

	//Set its relative position and random rotations about us
	branch->GetPhysicsNode().SetPosition(branchPos);

	branch->GetPhysicsNode().SetBroadPhaseVolume(new CollisionSphere(branchPos, growthAmount * maxTreeHeight));
	branch->GetPhysicsNode().SetNarrowPhaseVolume(new CollisionConvex(cylinder));

	//An orientation... not sure if this is correct just yet!
	branch->GetPhysicsNode().SetOrientation(
		Quaternion::FromMatrix(
		Matrix4::Rotation((float) 20 +(rand() % 35), Vector3(RAND()- RAND(),0,RAND()-RAND()))));

	//We dont want the object to scale with us, only scale its position
	/*branch->SetScaleWithParent(false);*/
	branch->GetPhysicsNode().SetFixed(true);
	branch->ConnectToSystems();
	this->AddChild(branch);

	//Will this branch bear fruit?
	/*if (rand()% ONE_OVER_FRUIT == 0 && branch->depth == MAX_DEPTH){
		branch->fruitBearing = true;
	}*/
}

void TreeEntity::AddRandomLeaf(){
	//Add a random transparent leaf
	//float leafScale = (1.0f/60.0f) * maxTreeHeight;
	float leafScale = 20.0f;

	GrowingNode* leaf = new GrowingNode(Vector3(leafScale, leafScale, leafScale), LEAF_GROW_TIME, Vector4(1,1,1,0.5f));

	leaf->SetMesh(leafQuad);

	//leaf->SetShader(fruitShader);
	//leaf->SetUpdateShaderFunction([this]{ this->LeafShaderUpdate(); });

	//At a random postion along the branch
	/*leaf->SetPosition(Vector3(0, RAND(), 0));*/
	float xOffset = RAND();
	leaf->SetTransform(Matrix4::Translation(Vector3(xOffset * 10.0f, RAND() * maxTreeHeight / 2, (1 - xOffset) * 10.0f)) *
		Matrix4::Rotation((float) 20 +(rand() % 35), Vector3(RAND()- (2  * RAND()),0,RAND()- (2 * RAND()))));
	//leaf->SetModelRotation((float) 20 +(rand() % 35), Vector3(RAND()- (2  * RAND()),0,RAND()- (2 * RAND())));

	//Vector3 axisOfGrowth = Vector3::Cross(leaf->GetPosition(), leaf->GetRotationAxis());

	//leaf->SetPosition(leaf->GetPosition() + (axisOfGrowth * (1-leaf->GetPosition().y)));

	leaf->SetBoundingRadius(25);

	//leaf->SetScaleWithParent(false);

	//this->AddChild(leaf);
	this->renderNode->AddChild(leaf);
}