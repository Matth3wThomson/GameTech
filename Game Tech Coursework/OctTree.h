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

//1 means collapse the tree every frame, 100 means collapse the tree once per 100 frames
#define NO_FRAMES_TILL_COLLAPSE 300

//TODO: Consider using a free list of octNodes to improve cache efficiency.
//TODO: Consider using a boolean to represent whether an octNode has octNodes
//			or physics nodes. Size() function could be expensive!
//TODO: Take octNodes off of the heap, why are they on the heap in the first place?
//			-Possibly due to vectors nullifying pointers to elements when the vector is inserted to 
//			 or deleted from? Why use a vector for the octNodes, we know there will always be 8!

//TODO: Make deletion of awake nodes occur using a generic delete function which scours the tree
//		for each node?

//TODO: Look into move constructors as a way to improve the performance of the collapse function.
//THE ISSUE IS COLLAPSING THE TREE!

//TODO: Make this store pointers to collision volumes, not phyiscs nodes... as not all collision volumes
//		need a physics node?

//TODO: Make an erase function and then just have the physics system re insert all nodes... Either for 
//		awake nodes only, or all nodes.

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

	//vector<OctNode> octNodes; //TODO: UNCOMMENT THIS AND FIX ALL ERRORS

};

class Renderer;

class OctTree
{
public:

	friend class PhysicsSystem; //This allows our physics system to access our components
	friend class Renderer; //This is to allow our renderer to draw our octTree!

	//The octree will encompass -halfsize -> +halfSize, Allow up to threshold number of
	//objects in each partition, and will only recurse down to maxDepth
	OctTree(float halfSize, int threshold, int maxDepth, const Vector3& worldCenter = Vector3(0,0,0));
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

	int frameCounter;

	//TODO: REMOVE THIS:
	int maxNodesAware;

	//Used to give an octNode children.
	void CreateNodes(OctNode& node);

	//Creates an individual node based on a node number (8 children per node MAX)
	OctNode* CreateNode(int nodeNumber, OctNode& parent);

	void CollapseTree();

	//Used to remove all physics nodes from the supplied nodes children, then removes the
	//children from the node, and then emplaces removed nodes directly into the node supplied
	/*void CollapseNode(OctNode& node);*/
	/*bool CollapseNode(OctNode& node);*/

	set<PhysicsNode*> CollapseNode(OctNode& node);

	//Counts the number of unique physics nodes in the node supplied
	int CountUniquePhysicsNodesInChildren(const OctNode& node);

	//Works out what type of physics node we are dealing with, and calls the correct method
	//to insert the physicsNode into the octNode suppplied
	bool InsertPhysicsNode(OctNode& into, PhysicsNode* pn);

	bool InsertColPlaneNode(OctNode& into, const Plane& colSphere, PhysicsNode* pn);
	bool InsertColSphereNode(OctNode& into, const CollisionSphere& colSphere, PhysicsNode* pn);
	bool InsertColAABBNode(OctNode& into, const CollisionAABB& aabb, PhysicsNode* pn);

	//Function that removes all nodes not at rest from the supplied node, and returns the number
	//of physics nodes left in the node (after removal of awake nodes).
	void RemoveAwake(OctNode& root, set<PhysicsNode*>& removed);
};

