#pragma once

#include "GameEntity.h"
#include "GrowingNode.h"
#include "Renderer.h"

//#define MAX_TREE_HEIGHT 750.0f //The size of a tree
#define TOLERANCE 0.01f	//Tolerance value for comparing floats
#define MAX_DEPTH 1	//The recursion limit
#define MAX_LEAVES 12	//The number of leaves on the final branches
#define ONE_OVER_FRUIT 5 //The 1/x chance that a fruit is grown
#define LEAF_GROW_TIME 10000 //The time it takes for a leaf to get to full size
#define FRUIT_GROW_TIME 10000	//The time it takes for a fruit to get to full size

/*
	Adapted from the TreeNode class from the previouos coursework. A
	few changes have been made to make it suitable for the physics system.
*/
class TreeEntity : public GameEntity
{
public:
	TreeEntity(const float maxTreeHeight);
	virtual ~TreeEntity(void);

	virtual void Update(float msec);

	void ResetTree(); //Not necessary?

protected:
	TreeEntity(Mesh* m, int depth, float maxBranchHeight);

	float maxTreeHeight;

	//Growth functions
	bool stillGrowing;	

	//Function for growing the tree over time
	float TendToOne(float x);

	static float growthSpeed;
	float growthAmount; //The amount the tree has grown

	float lastBranch;	//The timestamp of the last branch to be added
	float branchInterval; //The interval to add branches

	float timePassed; //The amount of time that has passed

	//Recursion limits
	int depth;

	//Branches and leaves
	void AddRandomBranch();
	void AddRandomLeaf();
	void LeafShaderUpdate();
	
	int numLeaves;

	//Fruit
	void AddFruit(TreeEntity* n);
	void FruitShaderUpdate();

	bool fruitBearing;
	float fruitStartGrowth;
	bool fruitGrown;

	//Particles
	void AddParticleEmitter();

	//Because this class adds particle emitters with their own meshes
	//that dynamically increase in size over time, we have to have a method
	//that searches the tree for particle emitters to calculate their sizes
	unsigned int GetParticleEmitterSizes();

	//Trunk and branch mesh
	static Mesh* cylinder;
	static GLuint trunkTex;
	static GLuint trunkToonTex;

	//Leaves
	static Mesh* leafQuad;
	static GLuint leafTex;
	static GLuint leafToonTex;

	//Fruit
	static Mesh* fruitMesh;
	static Shader* fruitShader;

	//Particles
	static Shader* particleShader;
	static GLuint particleTex;

	//Because we have static pointers we must keep track of
	//the number of objects that are instantiated
	static int instantiatedObjects;

	//Memory used
	unsigned int GPUBytes;
};

