#include "OctTree.h"


OctTree::OctTree(const Vector3& size, int threshold, int maxDepth)
{
	root.size = size;
	root.pos = Vector3(0,0,0); //Changed from located about center to origin
	root.parent = NULL;
	
	CreateNodes(root);

	this->threshold = threshold;
	this->maxDepth = maxDepth;
}


OctTree::~OctTree(void)
{
}

OctNode* OctTree::CreateNode(int nodeNumber, OctNode& parent){
	OctNode* o = new OctNode();

	o->size = parent.size * 0.5;

	Vector3 position = parent.pos;

	bitset<32> b(nodeNumber);
	if (b.test(0)){
		position.z = (parent.pos.z + (o->size.z * 0.5f));
	}

	if (b.test(1)){
		position.y = (parent.pos.z + (o->size.z * 0.5f));
	}

	if (b.test(2)){
		position.x = (parent.pos.z + (o->size.z * 0.5f));
	}

	o->pos = position;
	o->parent = &parent;

	return o;
}

void OctTree::CreateNodes(OctNode& node){
	for (int i=0; i<8; ++i){
		node.octNodes.push_back(CreateNode(i, node));
	}
};

bool OctTree::AddPhysicsNode(PhysicsNode* pn){
	CollisionVolume* colVol = pn->GetCollisionVolume();
	CollisionVolumeType cvt = colVol->GetType();

	if (cvt == COLLISION_SPHERE){

	}

}