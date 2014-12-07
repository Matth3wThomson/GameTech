#pragma once

#include <vector>
#include <set>
#include <bitset>
#include "../nclgl/Vector3.h"
#include "../nclgl/Plane.h"
#include "PhysicsNode.h"
#include "Collision.h"

using std::vector;
using std::set;

//TODO: Consider using a free list of octNodes to improve cache efficiency.

//ALWAYS CUBES!
struct OctNode {
	OctNode* parent;

	int depth;

	float halfSize;
	Vector3 pos;

	//I could cheat and make phyics node a set... But that's extra overhead
	//when adding to a vector... I should just be able to make sure that
	//duplicates are not added!!!!!!!!
	vector<PhysicsNode*> physicsNodes;
	vector<OctNode*> octNodes;

};

class Renderer;

class OctTree
{
public:

	friend class PhysicsSystem; //This allows our physics system to access our components
	friend class Renderer; //This is to allow our renderer to draw our octTree!

	//The octree will encompass -halfsize -> +halfSize, Allow up to threshold number of
	//objects in each partition, and will only recurse down to maxDepth
	OctTree(float halfSize, int threshold, int maxDepth);
	~OctTree(void);

	//Adds a physics node to the octree. A reference to this node will be kept
	//for the length of the program.
	bool AddPhysicsNode(PhysicsNode* pn);

	//TODO: Remove physics Node

	//Updates the octree. This re-sorts all nodes not at rest or fixed.
	void Update();

protected:
	OctTree();

	OctNode root;
	int threshold;
	int maxDepth;

	//TODO: REMOVE THIS:
	int maxNodesAware;

	//Used to give an octNode children.
	void CreateNodes(OctNode& node);

	//Creates an individual node based on a node number (8 children per node MAX)
	OctNode* CreateNode(int nodeNumber, OctNode& parent);

	//Used to remove all physics nodes from the supplied nodes children, then removes the
	//children from the node, and then emplaces removed nodes directly into the node supplied
	void CollapseNode(OctNode& node);

	//Works out what type of physics node we are dealing with, and calls the correct method
	//to insert the physicsNode into the octNode suppplied
	bool InsertPhysicsNode(OctNode& into, PhysicsNode* pn);

	bool InsertColPlaneNode(OctNode& into, const Plane& colSphere, PhysicsNode* pn);
	bool InsertColSphereNode(OctNode& into, const CollisionSphere& colSphere, PhysicsNode* pn);
	bool InsertColAABBNode(OctNode& into, const CollisionAABB& aabb, PhysicsNode* pn);

	//Function that removes all nodes not at rest from the supplied node, and returns the number
	//of physics nodes left in the node (after removal of awake nodes).
	int RemoveAwake(OctNode& root, set<PhysicsNode*>& removed);
};

