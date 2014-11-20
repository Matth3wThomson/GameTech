#pragma once

#include "../nclgl/SceneNode.h"
#include "../nclgl/ParticleEmitter.h"
#include "../nclgl/OBJMesh.h"
#include "ParticleEmitterNode.h"
#include "GrowingNode.h"


//TODO: Could remove relative scaling, and just have relative positioning on branches!?

//Investigate why this isnt scaled accoring to a given scale...
#define MAX_TREE_HEIGHT 750.0f //The size of a tree
#define TOLERANCE 0.01f	//Tolerance value for comparing floats
#define MAX_DEPTH 2	//The recursion limit
#define MAX_LEAVES 12	//The number of leaves on the final branches
#define ONE_OVER_FRUIT 5 //The 1/x chance that a fruit is grown
#define LEAF_GROW_TIME 10000 //The time it takes for a leaf to get to full size
#define FRUIT_GROW_TIME 10000	//The time it takes for a fruit to get to full size

//TODO: Remove relative pos
class TreeNode : public SceneNode
{
public:
	//The constructor used for creating the root of the tree (or trunk)
	TreeNode(Shader* particleShader, Shader* fruitShader);

	virtual ~TreeNode(void);

	virtual void Update(float msec);
	virtual void Draw(OGLRenderer& r, const bool useShader = true);

	unsigned int GetGPUMemUsage();
	float GetGrowthAmount(){ return growthAmount; }
	void SwitchToToon(bool toon);
	void ResetTree();

protected:

	//The constructor used for creating a child of the tree.
	TreeNode(Mesh* m, int depth);

	//Growth functions
	bool stillGrowing;	

	float TendToOne(float x);
	static float growthSpeed;
	float growthAmount;

	float lastBranch;
	float branchInterval;

	float timePassed;

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

