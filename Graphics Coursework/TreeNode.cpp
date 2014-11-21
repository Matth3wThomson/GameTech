#include "TreeNode.h"

int TreeNode::instantiatedObjects = 0;

//Branches
Mesh* TreeNode::cylinder = NULL;
GLuint TreeNode::trunkTex = 0;
GLuint TreeNode::trunkToonTex = 0;

//Leaves
float TreeNode::growthSpeed = 0.9f; //Higher means slower!
Mesh* TreeNode::leafQuad = NULL;
GLuint TreeNode::leafTex = 0;
GLuint TreeNode::leafToonTex = 0;

//Static fruit members
Mesh* TreeNode::fruitMesh = NULL;
Shader* TreeNode::fruitShader = NULL;

//Particle emission
Shader* TreeNode::particleShader = NULL;
GLuint TreeNode::particleTex = 0;


TreeNode::TreeNode(Shader* pShader, Shader* fruitShad)
{
	GPUBytes = 0;
	instantiatedObjects++;

	//Trunk + Branches
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

		particleShader = pShader;

		glBindTexture(GL_TEXTURE_2D, cylinder->GetTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindTexture(GL_TEXTURE_2D, cylinder->GetBumpMap());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);

	}

	mesh = cylinder;

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

		fruitShader = fruitShad;

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

	stillGrowing = true; 

	depth = 0;
	timePassed = 0;
	branchInterval = 0.1f;
	numLeaves = 0;
	lastBranch = branchInterval;

	fruitBearing = false;
	fruitGrown = true;
	fruitStartGrowth = 0;
}

TreeNode::TreeNode(Mesh* m, int depth){

	mesh = m;

	this->depth = depth;
	stillGrowing = true;
	timePassed = 0;
	numLeaves = 0;
	branchInterval = 0.05f;
	lastBranch = branchInterval;

	fruitBearing = false;
	fruitGrown = true;
	fruitStartGrowth = 0;
}


TreeNode::~TreeNode(void)
{
	//Then this is the base of the tree!
	if (depth == 0){
		if (--instantiatedObjects == 0){
			delete fruitMesh;
			delete leafQuad;
			delete cylinder;
		};
	}
}

void TreeNode::Update(float msec){
	timePassed += msec/1000; //Convert to seconds (For performance on tendToOne equation)

	if (stillGrowing){
		growthAmount = TendToOne(timePassed);

		if ( (1 - growthAmount) < TOLERANCE){
			if (fruitBearing){
				//Set fruit growing...
				fruitStartGrowth = timePassed;
				AddFruit(this);
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
				AddParticleEmitter();
			}
		}
	}

	//If this is the root node, scale it accordingly!
	if (this->depth == 0){
		this->SetModelScale(Vector3(growthAmount*0.05f * MAX_TREE_HEIGHT,
			growthAmount * MAX_TREE_HEIGHT,
			growthAmount* 0.05f * MAX_TREE_HEIGHT));
	} else {
		float baseScale = max(0.1f, growthAmount * (1-position.y) *2 );

		modelScale = Vector3(baseScale * 10, growthAmount * MAX_TREE_HEIGHT, baseScale * 10);
	}

	SetBoundingRadius(growthAmount  * MAX_TREE_HEIGHT);

	//Dont endlessly grow the tree....
	if (depth < MAX_DEPTH){
		//Add branches if it this node is not at the branch limit
		if (growthAmount * (depth+1) > lastBranch){
			lastBranch+= branchInterval;
			AddRandomBranch();
		}
	} else if (depth == MAX_DEPTH) {
		//Else add leaves!
		if (numLeaves < MAX_LEAVES){
			AddRandomLeaf();
			numLeaves++;
		}
	}

	SceneNode::Update(msec);
}

void TreeNode::Draw(OGLRenderer& r, const bool useShader){
	SceneNode::Draw(r, useShader);
}

void TreeNode::AddRandomBranch(){

	//Create another tree node with a depth value of this+1
	TreeNode* branch = new TreeNode(mesh, depth+1);

	branch->SetShader(shader);
	branch->SetUpdateShaderFunction(updateShaderFunction);

	//Set its relative position and random rotations about us
	branch->SetPosition(Vector3(0, growthAmount * (depth+1), 0));

	branch->SetModelRotation((float) 20 +(rand() % 35), Vector3(RAND()- RAND(),0,RAND()-RAND()));

	//We dont want the object to scale with us, only scale its position
	branch->SetScaleWithParent(false);

	this->AddChild(branch);

	//Will this branch bear fruit?
	if (rand()% ONE_OVER_FRUIT == 0 && branch->depth == MAX_DEPTH){
		branch->fruitBearing = true;
	}
}

void TreeNode::AddFruit(TreeNode* n){

	//Add a fruit to the branch
	n->fruitBearing = true;
	n->fruitGrown = false;

	float fruitScale = (1.0f/60.0f) * MAX_TREE_HEIGHT;

	GrowingNode* fruit = new GrowingNode(Vector3(fruitScale,fruitScale,fruitScale), 10000, Vector4(1,1,1,1));

	fruit->SetMesh(fruitMesh);
	fruit->SetShader(fruitShader);
	fruit->SetUpdateShaderFunction([this]{ this->FruitShaderUpdate(); });

	//Put the fruit at the end of the branch
	fruit->SetPosition(Vector3(0,1,0));

	fruit->SetScaleWithParent(false);

	n->AddChild(fruit);
}

void TreeNode::AddRandomLeaf(){

	//Add a random transparent leaf
	float leafScale = (1.0f/60.0f) * MAX_TREE_HEIGHT;

	GrowingNode* leaf = new GrowingNode(Vector3(leafScale, leafScale, leafScale), LEAF_GROW_TIME, Vector4(1,1,1,0.5));

	leaf->SetMesh(leafQuad);

	leaf->SetShader(fruitShader);
	leaf->SetUpdateShaderFunction([this]{ this->LeafShaderUpdate(); });

	//At a random postion along the branch
	leaf->SetPosition(Vector3(0, RAND(), 0));

	leaf->SetModelRotation((float) 20 +(rand() % 35), Vector3(RAND()- (2  * RAND()),0,RAND()- (2 * RAND())));

	Vector3 axisOfGrowth = Vector3::Cross(leaf->GetPosition(), leaf->GetRotationAxis());

	leaf->SetPosition(leaf->GetPosition() + (axisOfGrowth * (1-leaf->GetPosition().y)));

	leaf->SetBoundingRadius(25);

	leaf->SetScaleWithParent(false);

	this->AddChild(leaf);
}

void TreeNode::AddParticleEmitter(){

	//Adds a new particle emitter to the tree, with its own
	// particle emitter mesh
	ParticleEmitterNode* pNode = new ParticleEmitterNode();
	ParticleEmitter* pe = new ParticleEmitter();
	pe->SetTexture(particleTex);
	pe->SetParticleSize(5.0f);
	pe->SetParticleSpeed(0.05f);
	pe->SetColourVariance(0.0f);

	pNode->SetBoundingRadius(growthAmount * 0.2f * MAX_TREE_HEIGHT);
	pNode->SetParticleEmitter(pe);
	pNode->SetShader(particleShader);
	pNode->SetModelScale(Vector3(1,0.5,1));
	pNode->SetPosition(Vector3(0,1,0));

	this->AddChild(pNode);
}

unsigned int TreeNode::GetGPUMemUsage(){
	return GPUBytes + GetParticleEmitterSizes();
}

unsigned int TreeNode::GetParticleEmitterSizes(){
	unsigned int particleEmitterGPUMem = 0;

	//Loop through your own children and obtain GPU memory used from your own children
	for (auto itr = children.begin(); itr != children.end(); ++itr){
		ParticleEmitterNode* pe = dynamic_cast<ParticleEmitterNode*>((*itr));

		if (pe)
			particleEmitterGPUMem += pe->GetParticleEmitter()->GetGPUMemoryUsed();
	}

	for (auto itr = children.begin(); itr != children.end(); ++itr){
		TreeNode* tn = dynamic_cast<TreeNode*>((*itr));

		if (tn)
			particleEmitterGPUMem += tn->GetParticleEmitterSizes();
	}

	return particleEmitterGPUMem;
}

float TreeNode::TendToOne(float x){
	return -pow( growthSpeed, x )+ 1;
}

void TreeNode::FruitShaderUpdate(){
	glUniform1i(glGetUniformLocation(fruitShader->GetProgram(),
		"useTex"), false);
}

void TreeNode::LeafShaderUpdate(){
	glUniform1i(glGetUniformLocation(fruitShader->GetProgram(),
		"useTex"), true);
}

void TreeNode::ResetTree(){
	//Need to delete all nodes below this
	for (auto itr = children.begin(); itr != children.end(); ++itr){
		delete (*itr);
	}

	children.clear();

	//Then simply reset our values
	stillGrowing = true;

	depth = 0;
	timePassed = 0;
	branchInterval = 0.1f;
	numLeaves = 0;
	lastBranch = branchInterval;

	fruitBearing = false;
	fruitGrown = true;
	fruitStartGrowth = 0;
}

void TreeNode::SwitchToToon(bool toon){
	if (toon){
		cylinder->SetTexture(trunkToonTex);
		leafQuad->SetTexture(leafToonTex);
	} else {
		cylinder->SetTexture(trunkTex);
		leafQuad->SetTexture(leafTex);
	}
}