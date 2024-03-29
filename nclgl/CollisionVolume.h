#pragma once

#include "Mesh.h"
#include "OBJMesh.h"

class CollisionData {
	public:
	Vector3 m_point;
	Vector3 m_normal;
	float m_penetration;
};


enum CollisionVolumeType {
	COLLISION_PLANE,
	COLLISION_SPHERE,
	COLLISION_AABB,
	COLLISION_CONVEX
	
};

class Line {
public:
	Line(const Vector3& pos1, const Vector3& pos2):
		m_pos1(pos1), m_pos2(pos2){ };

	Vector3 m_pos1;
	Vector3 m_pos2;
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

//NOT TO BE USED AS A NARROW PHASE COLLISION VOLUME
class CollisionAABB : public CollisionVolume {
public:

	CollisionAABB(){ type = COLLISION_AABB; }
	CollisionAABB(Vector3& pos, Vector3& halfSize):
		m_position(pos), m_halfSize(halfSize){ type = COLLISION_AABB; }
	virtual ~CollisionAABB(){};
	CollisionAABB(Mesh* m){ GenerateAABB(m); };

	Vector3 m_position;
	Vector3 m_halfSize;

protected:
	//Generates an AABB from a mesh
	void GenerateAABB(Mesh* m);
};

//Collision convex is a mesh of points that represent a shape, stored in any arbitrary order.
//These can be (and should be for drawing purposes) loaded in from a mesh file. 
//NOT TO BE USED AS A BROAD PHASE VOLUME
class CollisionConvex : public CollisionVolume {
public:

	CollisionConvex(Mesh* m);

	void Update(const Vector3& position, const Quaternion& orientation, const Vector3& scale);

	Vector3 m_pos;
	
	Mesh* m_mesh;
	Vector3* m_collisionMesh;
	int m_numVertices;
};

