#include "OctTree.h"

OctTree::OctTree(float halfSize, int threshold, int maxDepth, const Vector3& worldCenter)
{
	root.halfSize = halfSize;
	root.pos = worldCenter;
	root.parent = NULL;
	root.depth = 0;

	CreateNodes(root);

	this->threshold = threshold;
	this->maxDepth = maxDepth;
	frameCounter = 0;
}


OctTree::~OctTree(void)
{
	CollapseNode(root);
}

//PUBLIC FUNCTIONS
bool OctTree::AddPhysicsNode(PhysicsNode* pn){
	return InsertPhysicsNode(root, pn);
}


void OctTree::Update(){

	frameCounter++;

	set<PhysicsNode*> awakeNodes;

	RemoveAwake(root, awakeNodes);

	//Re insert all of the nodes that were found to be awake
	for (auto itr = awakeNodes.begin(); itr != awakeNodes.end(); ++itr){
		InsertPhysicsNode(root, *itr);
	}

	//Collapse the tre every so often!
	if (frameCounter > NO_FRAMES_TILL_COLLAPSE){
		CollapseTree();
		frameCounter = 0;
	}
}

//Method that creates octNode children in the supplied octNode
void OctTree::CreateNodes(OctNode& node){
	for (int i=0; i<8; ++i){
		node.octNodes.push_back(CreateNode(i, node));
	}
};

//Create a node based on its node number (0-7) using bitwise operators
//we can nicely make a for loop out of creating nodes
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


void OctTree::CollapseTree(){
	CollapseNode(root);
}


set<PhysicsNode*> OctTree::CollapseNode(OctNode& node){

	//We have octNodes for children, try and collapse our children
	if (node.octNodes.size() != 0){

		//Work out how many unique physics nodes there are in all of our children.
		std::set<PhysicsNode*> pns;

		//Loop through all of our octNode children
		for (auto itr = node.octNodes.begin(); itr != node.octNodes.end(); ++itr){

			//Try to collapse the children in each node, and obtain all of 
			//the unique nodes in each
			set<PhysicsNode*>& pn = CollapseNode(**itr);
			pns.insert(pn.begin(), pn.end());

		}
		
		//Once we have tried to collapse all of our children, we then work out if we can
		//be collapsed. If not we tell our parent that this we aren't collapsable
		//by returning the set of all of our nodes.
		if (pns.size() > threshold){ return pns; };

		//If we have reached this point, the sum of our childrens physics nodes must be
		//less than or equal to the threshold.

		//Loop through all the unique physics nodes we found in our children,
		//and insert them into ourself if we are lower than the threshold!
		for (auto itr = pns.begin(); itr != pns.end(); ++itr){
			node.physicsNodes.push_back(*itr);
		}

		//Then remove our old octNodes
		while (!node.octNodes.empty()){
			delete node.octNodes.back();
			node.octNodes.pop_back();
		}

		//Then let our parent know how many physics nodes we contain
		return pns;
	}
	//We have physicsNodes for children, add them all to a set and send them to our parent
	else {

		std::set<PhysicsNode*> pns;

		for (auto itr = node.physicsNodes.begin(); itr != node.physicsNodes.end(); ++itr){
			pns.insert(*itr);
		}

		return pns;
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

		if (!InsertColSphereNode(into, *cs, pn)){
			std::cout << "A sphere failed to be inserted!!!! :(" << std::endl;
			return false;
		};
		return true;
	} else if (cvt == COLLISION_PLANE){
		Plane* p = (Plane*) colVol;
		pn->UpdateCollisionPlane(*p);

		return InsertColPlaneNode(into, *((Plane*) colVol), pn);
	} else if (cvt == COLLISION_AABB){

		CollisionAABB* aabb = (CollisionAABB*) colVol;
		pn->UpdateCollisionAABB(*aabb);

		return InsertColAABBNode(root, *aabb, pn);
	}

	std::cout << "Broad phase type was unknown. ";
	return false;
}

//Attempt at insertion of a collision plane into a octNode, based on treating the octNode
//as an AABB.
bool OctTree::InsertColPlaneNode(OctNode& into, const Plane& colPlane, PhysicsNode* pn){

	if (!Collision::AABBInColPlane(colPlane, into.pos, Vector3(into.halfSize, into.halfSize, into.halfSize)))
		return false;

	bool inserted = false;

	if (into.octNodes.size() != 0){
		for (auto itr = into.octNodes.begin(); itr != into.octNodes.end(); ++itr){
			if (InsertColPlaneNode(**itr, colPlane, pn)) inserted = true;
		}

		if (!inserted){
			std::cout << "The node should be in this parent, but was not inserted correctly!" << std::endl;
		}

	} else if (into.physicsNodes.size() == threshold && into.depth < maxDepth){

		CreateNodes(into);

		set<PhysicsNode*> toBeReinserted;

		//Re-sort each physics node back into this node's children.
		while (!into.physicsNodes.empty()){

			if (!InsertPhysicsNode(into, into.physicsNodes.back())){
				std::cout << "Putting a physics node back into the tree failed after a reshuffle!";
			}

			into.physicsNodes.pop_back();
		}

		//Finally try inserting our physics node into the newly created children of this node
		for (auto itr = into.octNodes.begin(); itr != into.octNodes.end(); ++itr){
			if (InsertColPlaneNode(**itr, colPlane, pn)) inserted = true;
		}

		if (!inserted){
			std::cout << "The node should be in this parent, but was not inserted correctly!" << std::endl;
		}

	} else {
		into.physicsNodes.push_back(pn);
		

		if (into.physicsNodes.size() > 8) {
			std::cout << "THIS SHOULDNT HAVE HAPPENED! " << std::endl;
		}
		inserted = true;
	}

	if (!inserted){
		std::cout << "The node should be in this parent, but was not inserted correctly!" << std::endl;
		return false;
	}

	return true;
}

bool OctTree::InsertColSphereNode(OctNode& into, const CollisionSphere& colSphere, PhysicsNode* pn){

	//Using the collision class, if the sphere and the AABB constructed from the octnode
	//do not collide, then the sphere isnt in the node!
	if (!Collision::SphereAABBCollision(colSphere,
		CollisionAABB(into.pos, Vector3(into.halfSize, into.halfSize, into.halfSize)))){
			return false;
	}

	bool inserted = false;

	//If this OctNode has nodes for children then continue the recursive check in each child.
	if (into.octNodes.size() != 0){

		for (auto itr = into.octNodes.begin(); itr != into.octNodes.end(); ++itr){
			if (InsertColSphereNode(**itr, colSphere, pn)) inserted = true; 
		}

		//If the current node was at threshold, and is still below the max depth, then
		//split it down further and resort all the nodes
	} else if (into.physicsNodes.size() == threshold && into.depth < maxDepth){

		//Create oct nodes for the current octNode
		CreateNodes(into);

		//Re-sort each physics node into the newly created children
		while (!into.physicsNodes.empty()){

			if (!InsertPhysicsNode(into, into.physicsNodes.back())){ 
				std::cout << "Putting a physics node back into the tree failed after a reshuffle!";
			}

			into.physicsNodes.pop_back();
		}

		//Finally try inserting our physics node into the newly created children of this node
		for (auto itr = into.octNodes.begin(); itr != into.octNodes.end(); ++itr){
			if (InsertColSphereNode(**itr, colSphere, pn)) inserted = true;
		}

	} else {
		//This octNode needs no resorting, so simply insert the physics Node
		into.physicsNodes.push_back(pn);
		inserted = true;
	}

	if (!inserted){
		std::cout << "The node should be in this parent, but was not inserted correctly!" << std::endl;
		return false;
	}

	//THIS IS AN ASSUMPTION THAT THIS FUNCTION WORKS
	//The object has successfully been placed in this child somewhere...
	return true;
}

bool OctTree::InsertColAABBNode(OctNode& into, const CollisionAABB& aabb, PhysicsNode* pn){

	if (!Collision::AABBCollision(aabb, 
		CollisionAABB(into.pos, Vector3(into.halfSize, into.halfSize, into.halfSize)))){
			return false;
	}

	bool inserted = false;

	//If this OctNode has nodes for children then continue the recursive check in each child.
	if (into.octNodes.size() != 0){

		for (auto itr = into.octNodes.begin(); itr != into.octNodes.end(); ++itr){
			if (InsertColAABBNode(**itr, aabb, pn)) inserted = true; 
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

			if (!InsertPhysicsNode(into, into.physicsNodes.back())){ 
				std::cout << "Putting a physics node back into the tree failed after a reshuffle!";
			}

			into.physicsNodes.pop_back();
		}

		//Finally try inserting our physics node into the newly created children of this node
		for (auto itr = into.octNodes.begin(); itr != into.octNodes.end(); ++itr){
			if (InsertColAABBNode(**itr, aabb, pn)) inserted = true;
		}

	} else {
		//This octNode needs no resorting, so simply insert the physics Node
		into.physicsNodes.push_back(pn);
		inserted = true;
	}

	if (!inserted){
		std::cout << "The node should be in this parent, but was not inserted correctly!" << std::endl;
		return false;
	}

	//THIS IS AN ASSUMPTION THAT THIS FUNCTION WORKS
	//The object has successfully been placed in this child somewhere...
	return true;
}


//BROAD PHASE REMOVAL
void OctTree::RemoveAwake(OctNode& from, set<PhysicsNode*>& removedPNodes){

	//If the node supplied has physics nodes as children
	if (from.physicsNodes.size() != 0){

		//Find all nodes not at rest or fixed in place,
		//and add them to the removed set.
		for (auto itr = from.physicsNodes.begin(); itr != from.physicsNodes.end();){
			if ( !(*itr)->AtRest() && !(*itr)->GetFixed() ){

				removedPNodes.insert(*itr);

				itr = from.physicsNodes.erase(itr);
			} else	++itr;
		}
	}
	//If the node supplied has octNodes for children.
	else {
		//Count the number of nodes left in the children after removing all of the awake ones
		int x = 0;

		//Remove all of the awake nodes from the octNode children, and count the number of objects
		//that remain in the children. THIS NUMBER IS NOT REPRESENTITIVE OF UNIQUENESS...
		for (auto itr = from.octNodes.begin(); itr != from.octNodes.end(); ++itr){
			RemoveAwake((**itr), removedPNodes);
		}
	}
}