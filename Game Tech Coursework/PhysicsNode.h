/******************************************************************************
Class:PhysicsNode
Implements:
Author:Rich Davison	<richard.davison4@newcastle.ac.uk> and YOU!
Description: This class represents the physical properties of your game's
entities - their position, orientation, mass, collision volume, and so on.
Most of the first few tutorials will be based around adding code to this class
in order to add correct physical integration of velocity / acceleration etc to
your game objects. 


In addition to the tutorial code, this class contains a pointer to a SceneNode.
This pointer is to the 'graphical' representation of your game object, just 
like the SceneNode's used in the graphics module. However, instead of 
calculating positions etc as part of the SceneNode, it should instead be up
to your 'physics' representation to determine - so we calculate a transform
matrix for your SceneNode here, and apply it to the pointer. 

Your SceneNode should still have an Update function, though, in order to
update any graphical effects applied to your object - anything that will end
up modifying a uniform in a shader should still be the job of the SceneNode. 

Note that since the SceneNode can still have children, we can represent our
entire CubeRobot with a single PhysicsNode, and a single SceneNode 'root'.

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////


#pragma once

#include "../nclgl/Quaternion.h"
#include "../nclgl/Vector3.h"
#include "../nclgl/Matrix4.h"	//TODO: Remove this once matrix3 works.
#include "../nclgl/Matrix3.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/CollisionVolume.h"
#include "../nclgl/Plane.h"

#define FORCE_REST_TOLERANCE 0.001f
#define REST_TOLERANCE 0.01f
#define DAMPING_FACTOR 0.99f;

class PhysicsSystem;

//TODO: Clean this class up once matrix 3 conversion works!

class PhysicsNode	{
public:
	PhysicsNode(void);
	PhysicsNode(const Quaternion& orientation, const Vector3& position);
	~PhysicsNode(void);

	friend class PhysicsSystem;

	void SetInvSphereInertiaMatrix(float mass, float radius);
	void SetInvCuboidInertiaMatrix(float mass, float height, float width, float length);

	Vector3		GetPosition()const			{ return m_position;}
	void		SetPosition(const Vector3& pos){ m_position = pos; }

	Vector3		GetLinearVelocity()		{ return m_linearVelocity;}
	void		SetLinearVelocity(const Vector3& lv){ m_linearVelocity = lv; m_rest = false; };

	/*void		SetInverseInertiaMat(const Matrix4& m){ m_invInertia = m; }
	Matrix4		GetInverseInertiaMat(){ return m_invInertia; };*/

	void		SetInverseInertiaMat(const Matrix3& m){ m_invInertia = m; }
	Matrix3		GetInverseInertiaMat(){ return m_invInertia; }
	
	float		GetMass() const			{ return 1 / m_invMass; }
	void		SetMass(float mass)		{ m_invMass = 1 / mass; }
	float		GetInverseMass() const	{return m_invMass;}

	Quaternion	GetOrientation() const	{ return m_orientation;}
	void		SetOrientation(const Quaternion& orient){ m_orientation = orient; };

	Vector3		GetAngularVelocity()	{ return m_angularVelocity;}

	bool		AtRest() const			{ return m_rest; };

	bool		GetFixed() const		{ return fixed; }
	void		SetFixed(bool fix)		{ fixed = fix; if (fixed){ m_linearVelocity = Vector3(0,0,0); m_angularVelocity = Vector3(0,0,0); } };

	Matrix4		BuildTransform();

	//Consider removing this function for a centralized approach? Or remove interpolation from here?
	virtual void		Update(float msec);

	void	SetTarget(SceneNode *s) { target = s;}

	void SetNarrowPhaseVolume(CollisionVolume* cv){ m_narrowPhase = cv; if (!m_broadPhase) m_broadPhase = cv; };
	CollisionVolume* GetNarrowPhaseVolume(){ return m_narrowPhase; }

	void SetBroadPhaseVolume(CollisionVolume* cv){ m_broadPhase = cv; if (!m_narrowPhase) m_narrowPhase = cv; };
	CollisionVolume* GetBroadPhaseVolume(){ return m_broadPhase; }

	void UpdateCollisionSphere(CollisionSphere& cs);
	void UpdateCollisionPlane(Plane& p);

	void ApplyForce(const Vector3& force, const Vector3& distanceFromCentre = Vector3(0,0,0));
	void ApplyImpulse(const Vector3& impulse, const Vector3& distanceFromCentre = Vector3(0,0,0));

	Vector3	GetForce()	{ return m_force;}
	Vector3	GetTorque() { return m_torque;}

protected:
	bool fixed;
	bool m_rest;
	
	PhysicsNode* lastCollided;

	//<---------LINEAR-------------->
	Vector3		m_position;
	Vector3		m_linearVelocity;
	Vector3		m_force;
	float		m_invMass;	//No actual mass? Calculation of acceleration?

	//<----------ANGULAR--------------->
	Quaternion  m_orientation;
	Vector3		m_angularVelocity;
	Vector3		m_torque;
	//Matrix4     m_invInertia;   
	Matrix3		m_invInertia;

	//<----------COLLISION------------->
	CollisionVolume* m_broadPhase;
	CollisionVolume* m_narrowPhase;

	SceneNode*	target;  
};

