/******************************************************************************
Class:PhysicsSystem
Implements:
Author:Rich Davison	<richard.davison4@newcastle.ac.uk> and YOU!
Description: A very simple physics engine class, within which to implement the
material introduced in the Game Technologies module. This is just a rough 
skeleton of how the material could be integrated into the existing codebase -
it is still entirely up to you how the specifics should work. Now C++ and
graphics are out of the way, you should be starting to get a feel for class
structures, and how to communicate data between systems.

It is worth poinitng out that the PhysicsSystem is constructed and destructed
manually using static functions. Why? Well, we probably only want a single
physics system to control the entire state of our game world, so why allow 
multiple systems to be made? So instead, the constructor / destructor are 
hidden, and we 'get' a single instance of a physics system with a getter.
This is known as a 'singleton' design pattern, and some developers don't like 
it - but for systems that it doesn't really make sense to have multiples of, 
it is fine!

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////


#pragma once

#include "PhysicsNode.h"
#include "OctTree.h"
#include "Collision.h"
#include "Spring.h"
#include "../nclgl/Mesh.h"
#include "../nclgl/Plane.h"
#include <vector>
#include <mutex>

using std::vector;
using std::mutex;

/*
Rich: 
There are a few ways of integrating the collision volumes
discussed in this module into your little physics engine.
You could keep pointers to all types inside a physics node,
and check to see which ones are NULL. Or you could make all
collision volumes inherit from a base class, so we only need
a single pointer inside each Physics Node. We can then either
use a process of dynamic casting to determine which exact type
of volume the pointer is, or just cheat and use an enum member
variable (I do this in my own work, you can do whichever you
feel comfortable with!).

*/

//TODO: Possibly use a free list of collision volumes that can be obtained? Would mean
//		better cache efficiency?

//TODO: Remove all TODOS!

typedef std::pair<PhysicsNode*, PhysicsNode*> COLLISION_PAIR;

class PhysicsSystem	{
public:
	friend class GameClass;
	friend class Renderer; //This is to allow our renderer to be able to draw our physics entities!

	void		Update(float msec);

	void		BroadPhase();
	void		NarrowPhase();

	//Statics
	static void Initialise() {
		instance = new PhysicsSystem();
	}

	static void Destroy() {
		delete instance;
	}

	static PhysicsSystem& GetPhysicsSystem() {
		return *instance;
	}

	void WakeAllNodes();

	void	AddNode(PhysicsNode* n);
	void	RemoveNode(PhysicsNode* n);

	void	AddConstraint(Constraint* c);
	void	RemoveConstraint(Constraint* c);

	int		GetUpdateRate(){ return updateRate; };
	int		GetCollisionCount(){ return collisionCount; };
	unsigned int GetNumberOfObjects(){ return allNodes.size(); };

protected:
	PhysicsSystem(void);
	~PhysicsSystem(void);

	/*void NarrowPhaseTree(OctNode& on);*/
	void NextPhaseTree(OctNode& on, bool boundingBox = false);

	//Performs a narrowphase check on the nodes in the supplied vector. This is
	//mainly here for testing purposes of narrowphase with few objects, but can be used
	//if you wish to skip the bounding box object / object checks.
	void NarrowPhaseVector(std::vector<PhysicsNode*>& np);

	//Performs a bounding colvol check on the nodes in the supplied vector,
	//and adds pairs of objects found to collide to the collisionPairs set
	void BroadPhaseVector(std::vector<PhysicsNode*>& np);

	//Method for updating all of the broad phase collision volumes of all
	//physics nodes in the system.
	void		UpdateBroadPhaseCollisionVolumes();

	//Performs a narrow phase check on all entities in the collision pairs set,
	//and adds collision response before clearing the set for the next frame
	void NarrowPhasePairs();

	static void AddCollisionImpulse( PhysicsNode& pn0, PhysicsNode& pn1, const CollisionData& cd);


	//Statics
	static PhysicsSystem* instance;

	//Awareness of the timestep being used throughout the physics engine
	static float timestep;

	//Counting the actual update rate of the physics engine
	GameTimer physTimer;
	float timePassed;
	int physFrames;
	int updateRate;

	int collisionCount;

	vector<PhysicsNode*> allNodes;
	mutex nodesMutex;

	vector<Constraint*> allConstraints;

	//Our broadphase checks!
	OctTree octTree;

	//This will be used to store all of the nodes that have been detected to collide.
	//There will then be a collision resolution step.
	std::set<std::pair<PhysicsNode*, PhysicsNode*>> collisionPairs;
};

