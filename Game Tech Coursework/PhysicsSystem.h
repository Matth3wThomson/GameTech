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


class PhysicsSystem	{
public:
	friend class GameClass;
	friend class Renderer; //This is to allow our renderer to be able to draw our physics entities!

	void		Update(float msec);

	void		UpdateCollisionVolume(PhysicsNode* pn);
	void		BroadPhase();
	void		NarrowPhase();
	void		ResolveCollisions();

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

	void	AddNode(PhysicsNode* n);

	void	RemoveNode(PhysicsNode* n);

	int		GetUpdateRate(){ return updateRate; };
	int		GetCollisionCount(){ return collisionCount; };

protected:
	PhysicsSystem(void);
	~PhysicsSystem(void);

	void NarrowPhaseTree(OctNode& on);
	void NarrowPhaseVector(std::vector<PhysicsNode*>& np);

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

	//Our broadphase checks!
	OctTree octTree;

	//This will be used to store all of the nodes that have been detected to collide.
	//There will then be a collision resolution step.
	vector<PhysicsNode*> collided;
};

