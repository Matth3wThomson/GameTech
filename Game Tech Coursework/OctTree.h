#pragma once

#include <vector>
#include <set>
#include <bitset>
#include "../nclgl/Vector3.h"
#include "PhysicsNode.h"

using std::vector;
using std::set;

//TODO: Consider using a free list of octNodes to improve cache efficiency.

struct OctNode {
	OctNode* parent;

	Vector3 size;

	Vector3 pos;

	vector<PhysicsNode*> physicsNodes;
	vector<OctNode*> octNodes;

};

class OctTree
{
public:
	OctTree(const Vector3& size, int threshold, int maxDepth);
	~OctTree(void);

	bool AddPhysicsNode(PhysicsNode* pn);

protected:
	OctNode root;
	int threshold;
	int maxDepth;

	void CreateNodes(OctNode& node);
	

	OctNode* CreateNode(int nodeNumber, OctNode& parent);
};

