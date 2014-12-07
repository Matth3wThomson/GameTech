#pragma once

#include "Vector3.h"
#include "CollisionVolume.h"

class PhysicsSystem;

class Plane : public CollisionVolume
{
public:

	friend class PhysicsSystem;
	friend class Collision;

	Plane(void){ type = COLLISION_PLANE; };
	Plane(const Vector3& normal, float distance, bool normalise =false);
	virtual ~Plane(void){};

	void SetNormal(const Vector3& normal){ this->m_normal = normal; }
	Vector3 GetNormal() const { return m_normal; }

	void SetDistance(float dist){ distance = dist; }
	float GetDistance() const{ return distance; }

	bool SphereInPlane(const Vector3& position, float radius, CollisionData* collisionData = NULL) const;


protected:
	Vector3 m_normal;
	float distance;

	
};

