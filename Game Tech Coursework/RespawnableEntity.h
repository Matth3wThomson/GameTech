#pragma once
#include "gameentity.h"

//An entity that will disappear after taking too many collisions,
//and respawn after a time supplied to the constructor
class RespawnableEntity :
	public GameEntity
{
public:
	RespawnableEntity(SceneNode* renderNode, PhysicsNode* physicsNode,
		const Vector3& respawnPos, float respawnTime, int collisionsToDie);
	virtual ~RespawnableEntity(void);

	virtual void Update(float msec);

protected:
	//Whether or not this entity is dead
	bool dead;

	//Where to respawn back to
	Vector3 m_respawnPos;

	//Number of collisions this entity can withstand
	int m_collisionsToDie;

	//How long has passed since the node died
	float m_timePassedSinceDeath;

	//How long it takes to respawn
	float m_respawnTimer;
};

