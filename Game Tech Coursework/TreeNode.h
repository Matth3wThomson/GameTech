#pragma once

#include "../nclgl/SceneNode.h"
#include "../nclgl/ParticleEmitter.h"
#include "../nclgl/OBJMesh.h"
#include "ParticleEmitterNode.h"
#include "GrowingNode.h"

/*
	NOTE:
		-Meshes used in this class are static to prevent reinstantiation of the same mesh.
			-In order to prevent multiple deletion, the class has a counter to count
			 the number of objects of this class that has been instantiated, and only
			 deletes these meshes if it is the last object to be deleted.
*/

//Investigate why this isnt scaled accoring to a given scale...
#define MAX_TREE_HEIGHT 750.0f //The size of a tree
#define TOLERANCE 0.01f	//Tolerance value for comparing floats
#define MAX_DEPTH 1	//The recursion limit
#define MAX_LEAVES 12	//The number of leaves on the final branches
#define ONE_OVER_FRUIT 5 //The 1/x chance that a fruit is grown
#define LEAF_GROW_TIME 10000 //The time it takes for a leaf to get to full size
#define FRUIT_GROW_TIME 10000	//The time it takes for a fruit to get to full size

class TreeNode : public SceneNode
{
public:
	
	TreeNode(Shader* particleShader, Shader* fruitShader);

	virtual ~TreeNode(void);

	virtual void Update(float msec);
	virtual void Draw(OGLRenderer& r, const bool useShader = true);

	unsigned int GetGPUMemUsage();
	float GetGrowthAmount(){ return growthAmount; }

	//Function to switch on toon textures that have been loaded in
	void SwitchToToon(bool toon);

	//Method to reset the tree to its beginning state
	void ResetTree();

protected:

	//The constructor used for creating a child of the tree.
	TreeNode(Mesh* m, int depth);

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
	int maxDepth;

	//Branches and leaves
	void AddRandomBranch();
	void AddRandomLeaf();
	void LeafShaderUpdate();
	
	int numLeaves;

	//Fruit
	void AddFruit(TreeNode* n);
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

