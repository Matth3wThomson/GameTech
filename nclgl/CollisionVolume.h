#pragma once

#include "Mesh.h"

class CollisionData {
	public:
	Vector3 m_point;
	Vector3 m_normal;
	float m_penetration;
};


enum CollisionVolumeType {
	COLLISION_PLANE,
	COLLISION_SPHERE,
	COLLISION_AABB
	
};

class CollisionVolume {
public:
	CollisionVolumeType GetType() { return type;}
	virtual ~CollisionVolume(){};

protected:
	CollisionVolumeType type;
};

class CollisionSphere : public CollisionVolume {
	public:
	CollisionSphere(const Vector3& position, float radius):
		m_pos(position), m_radius(radius){
			type = COLLISION_SPHERE; };
	virtual ~CollisionSphere(void){};

	Vector3 m_pos;
	float m_radius;
};

class CollisionAABB : public CollisionVolume {
public:

	CollisionAABB(){ type = COLLISION_AABB; }
	virtual ~CollisionAABB(){};
	CollisionAABB(Mesh* m){ GenerateAABB(m); };

	Vector3 m_position;
	Vector3 m_halfSize;

protected:
	//Generates an AABB from a mesh
	void GenerateAABB(Mesh* m);
};