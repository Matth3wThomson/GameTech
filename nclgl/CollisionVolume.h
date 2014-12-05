#pragma once

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