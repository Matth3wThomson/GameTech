#pragma once

#include "gameentity.h"
#include "Renderer.h"

//Entity that will break down into multiple smaller versions of itself upon experiencing too many
//collisions.
class DestructibleSphere :
	public GameEntity
{
public:
	DestructibleSphere(int collisionsToDie, Mesh* sphere, float radius, const Vector3& position, int subParts);
	virtual ~DestructibleSphere(void);

	virtual void Update(float msec);
protected:

	GameEntity* CreateSphereComponent(int totalParts);

	bool destroyed;
	int m_collisionsToDie;
	
	float radius;

	static Mesh* sphereMesh;
};

