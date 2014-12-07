#include "OctTree.h"

//TODO: Work out why a sphere fell through the floor... and work out why the octree is being
//split down further than is necessary
OctTree::OctTree(float halfSize, int threshold, int maxDepth)
{
	root.halfSize = halfSize;
	root.pos = Vector3(0,0,0); //Changed from located about center to origin
	root.parent = NULL;
	root.depth = 0;

	maxNodesAware = 0;

	CreateNodes(root);

	this->threshold = threshold;
	this->maxDepth = maxDepth;
}


OctTree::~OctTree(void)
{
}

//PUBLIC FUNCTIONS
bool OctTree::AddPhysicsNode(PhysicsNode* pn){
	return InsertPhysicsNode(root, pn);
}

//TODO: SORT OUT THIS PROBLEM!!! :/
void OctTree::Update(){

	set<PhysicsNode*> awakeNodes;

	RemoveAwake(root, awakeNodes);

	//TODO: Remove maxNodes aware stuff (resting nodes wont be removed!)
	if (maxNodesAware < awakeNodes.size()){ 
		maxNodesAware = awakeNodes.size();
		std::cout << "Nodes aware of: " << awakeNodes.size() << std::endl;
	} else if (awakeNodes.size() < maxNodesAware){
		//std::cout << "We lost track of a node!" << std::endl;
	}

	for (auto itr = awakeNodes.begin(); itr != awakeNodes.end(); ++itr){
		InsertPhysicsNode(root, *itr);
	}
}

//Method that creates octNode children in the supplied octNode
void OctTree::CreateNodes(OctNode& node){
	for (int i=0; i<8; ++i){
		node.octNodes.push_back(CreateNode(i, node));
	}
};

OctNode* OctTree::CreateNode(int nodeNumber, OctNode& parent){
	OctNode* o = new OctNode();

	o->halfSize = parent.halfSize * 0.5f;

	Vector3 position = parent.pos;

	bitset<32> b(nodeNumber);
	if (b.test(0)){
		position.z += o->halfSize;
	} else {
		position.z -= o->halfSize;
	}

	if (b.test(1)){
		position.y += o->halfSize;
	} else {
		position.y -= o->halfSize;
	}

	if (b.test(2)){
		position.x += o->halfSize;
	} else {
		position.x -= o->halfSize;
	}

	o->pos = position;
	o->parent = &parent;
	o->depth = parent.depth+1;

	return o;
}

//Collapses a node that has OctNodes for children
void OctTree::CollapseNode(OctNode& node){

	set<PhysicsNode*> toBeMoved;

	//For every octNode child
	for (auto itr = node.octNodes.begin(); itr != node.octNodes.end(); ++itr){

		//Obtain all unique objects in the children of the octNode
		while ( !(*itr)->physicsNodes.empty() ){
			toBeMoved.insert( (*itr)->physicsNodes.back() );
			(*itr)->physicsNodes.pop_back();
		}
	}

	//Now put all of those collated objects into the current octNode.
	for (auto itr = toBeMoved.begin(); itr != toBeMoved.end(); ++itr)
		node.physicsNodes.push_back(*itr);

	//Then delete all of the octNode children that were attached to this object
	while (!(node.octNodes.empty())){
		delete node.octNodes.back();
		node.octNodes.pop_back();
	}
}

//BROAD PHASE INSERTION
bool OctTree::InsertPhysicsNode(OctNode& into, PhysicsNode* pn){
	
	//We use a broad phase volume for storing objects into the octree!
	CollisionVolume* colVol = pn->GetBroadPhaseVolume();

	if (!colVol)
		return false;

	CollisionVolumeType cvt = colVol->GetType();

	if (cvt == COLLISION_SPHERE){
		CollisionSphere* cs = (CollisionSphere*) colVol;

		pn->UpdateCollisionSphere(*cs);

		//return InsertColSphereNode(into, *cs, pn);
		if (!InsertColSphereNode(into, *cs, pn)){
			std::cout << "A sphere failed to be inserted!!!! :(" << std::endl;
			return false;
		};
		return true;
	} else if (cvt == COLLISION_PLANE){
		//TODO: Update plane
		Plane* p = (Plane*) colVol;
		pn->UpdateCollisionPlane(*p);

		return InsertColPlaneNode(root, *((Plane*) colVol), pn);
	} else if (cvt == COLLISION_AABB){
		std::cout << "Unimplemented. ";
		return false;
	}

	std::cout << "Broad phase type was unknown. ";
	return false;
}

bool OctTree::InsertColSphereNode(OctNode& into, const CollisionSphere& colSphere, PhysicsNode* pn){
	//Check X Axis
	
	//See if highest point of sphere is below the lowest point of the node's size
	if ( (colSphere.m_pos.x + colSphere.m_radius) < (into.pos.x - into.halfSize) ){
		return false;
	}

	//See if the lowest point of the sphere is above the highest point of the nodes size
	if ( (colSphere.m_pos.x - colSphere.m_radius) > (into.pos.x + into.halfSize) ){
		return false;
	}

	//Check Y Axis
	if ( (colSphere.m_pos.y + colSphere.m_radius) < (into.pos.y - into.halfSize) ){
		return false;
	}

	if ( (colSphere.m_pos.y - colSphere.m_radius) > (into.pos.y + into.halfSize) ){
		return false;
	}

	//Check Z Axis
	if ( (colSphere.m_pos.z + colSphere.m_radius) < (into.pos.z - into.halfSize) ){
		return false;
	}

	if ( (colSphere.m_pos.z - colSphere.m_radius) > (into.pos.z + into.halfSize) ){
		return false;
	}

	//TODO: Spherical check is expensive... I could just omit this? Would mean it possible
	//for spheres to be within the bounds even though they arent (in rare cases...)
	//Is that a big deal for a broad phase cull though?

	//Passed "bounding box". Do a spherical check:
	//Distance between the center of the node and the centre of the colSphere
	//const float distSq = (colSphere.m_pos - into.pos).LengthSq();
//	const float distSq = (into.pos - colSphere.m_pos).LengthSq();
//	/*const float distSq = ((colSphere.m_pos.x - into.pos.x) * (colSphere.m_pos.x - into.pos.x)) +
//		((colSphere.m_pos.y - into.pos.y) * (colSphere.m_pos.y - into.pos.y)) +
//		((colSphere.m_pos.z - into.pos.z) * (colSphere.m_pos.z - into.pos.z));
//*/
//	const float cubeDiagonalSq = Vector3(into.halfSize, into.halfSize, into.halfSize).LengthSq();
//	//const float cubeDiagonal = sqrtf((into.halfSize * into.halfSize) + (into.halfSize * into.halfSize) + (into.halfSize * into.halfSize));
//
//	//const float sumRadius = (colSphere.m_radius + (into.pos + into.halfSize).Length());
//	//const float sumRadius = (colSphere.m_radius + (into.pos + cubeDiagonal).Length());
//	const float sumRadiusSq = (colSphere.m_radius * colSphere.m_radius) + cubeDiagonalSq;
//
//	//Check that the colSphere is within range.
//	/*if ( distSq > sumRadius * sumRadius)
//		return false;*/
//
//	if ( distSq > sumRadiusSq)
//		return false;

	//The current collision sphere is definitely in this octNode somewhere, now its just a case of
	//placing it either in the oct nodes or physics nodes.

	//If this OctNode has nodes for children then continue the recursive check in each child.
	if (into.octNodes.size() != 0){

		bool inserted = false;

		for (auto itr = into.octNodes.begin(); itr != into.octNodes.end(); ++itr){
			if (InsertColSphereNode(**itr, colSphere, pn)) inserted = true; 
		}

		if (!inserted)
			std::cout << "Unable to insert in children, although succesful to insert in parent!" << std::endl;
	
		//If the current node was at threshold, and is still below the max depth, then
		//split it down further and resort all the nodes
	} else if (into.physicsNodes.size() == threshold && into.depth < maxDepth){

		//Create oct nodes for the current octNode
		CreateNodes(into);

		//Re-sort each physics node
		while (!into.physicsNodes.empty()){
			PhysicsNode* pnTemp = into.physicsNodes.back();

			if (!InsertPhysicsNode(into, pnTemp)){ 
				std::cout << "Putting a physics node back into the tree failed after a reshuffle!";
			}

			into.physicsNodes.pop_back();
		}

		//Finally try inserting our physics node into the newly created children of this node
		for (auto itr = into.octNodes.begin(); itr != into.octNodes.end(); ++itr){
			InsertColSphereNode(**itr, colSphere, pn);
		}

	} else {
		//This octNode needs no resorting, so simply insert the physics Node
		into.physicsNodes.push_back(pn);
	}

	//THIS IS AN ASSUMPTION THAT THIS FUNCTION WORKS
	//The object has successfully been placed in this child somewhere...
	return true;
}

//TODO: Not complete!
//Attempt at insertion of a collision plane into a octNode, based on treating the octNode
//as an AABB.
bool OctTree::InsertColPlaneNode(OctNode& into, const Plane& colPlane, PhysicsNode* pn){
	//Get the distance from the edge of the box to the center
	Vector3 extense = (into.pos + into.halfSize) - into.pos;

	//Dot Product between the plane normal and the center of the axis aligned box
	float fRadius = abs(colPlane.GetNormal().x * extense.x) +
		abs(colPlane.GetNormal().y * extense.y) +
		abs(colPlane.GetNormal().z * extense.z);

	float dot = Vector3::Dot(colPlane.GetNormal(), into.pos) - colPlane.GetDistance();
		
	if (dot > fRadius) // PLANE is not in box!
		return false;

	if (into.octNodes.size() != 0){
		for (auto itr = into.octNodes.begin(); itr != into.octNodes.end(); ++itr){
			InsertColPlaneNode(**itr, colPlane, pn);
		}
	} else if (into.physicsNodes.size() == threshold && into.depth < maxDepth){

		CreateNodes(into);

		//Re-sort each physics node
		while (!into.physicsNodes.empty()){
			PhysicsNode* pnTemp = into.physicsNodes.back();

			if (!InsertPhysicsNode(into, pnTemp)){ 
				std::cout << "Putting a physics node back into the tree failed after a reshuffle!";
			}

			into.physicsNodes.pop_back();
		}

		//Finally try inserting our physics node into the newly created children of this node
		for (auto itr = into.octNodes.begin(); itr != into.octNodes.end(); ++itr){
			InsertColPlaneNode(**itr, colPlane, pn);
		}

	} else {
		into.physicsNodes.push_back(pn);
	}

	return true;
}

//BROAD PHASE REMOVAL
int OctTree::RemoveAwake(OctNode& from, set<PhysicsNode*>& removedPNodes){

	//If the node supplied has physics nodes as children
	if (from.physicsNodes.size() != 0){

		//Find all nodes not at rest and add them to the removed set.
		for (auto itr = from.physicsNodes.begin(); itr != from.physicsNodes.end();){
			if ( !(*itr)->AtRest() || !(*itr)->GetFixed() ){

				removedPNodes.insert(*itr);

				itr = from.physicsNodes.erase(itr);
			} else ++itr;
		}

		return from.physicsNodes.size();
	}
	//If the node supplied has octNodes for children.
	else {
		//Count the number of nodes left in the children after removing all of the awake ones
		int x = 0;
		
		//Remove all of the awake nodes from the octNode children, and count the number of spheres
		//they each contain
		for (auto itr = from.octNodes.begin(); itr != from.octNodes.end(); ++itr){
			x += RemoveAwake((**itr), removedPNodes);
		}

		//If the total number of spheres in all of the children is below the threshold,
		//we can collapse this node!
		if (x < threshold || x == 0){
			CollapseNode(from);
			return from.physicsNodes.size();
		}

		return x;
	}
}