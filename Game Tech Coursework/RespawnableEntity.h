#pragma once
#include "gameentity.h"
class RespawnableEntity :
	public GameEntity
{
public:
	RespawnableEntity(SceneNode* renderNode, PhysicsNode* physicsNode,
		const Vector3& respawnPos, float respawnTime, int collisionsToDie);
	virtual ~RespawnableEntity(void);

	virtual void Update(float msec);

protected:
	bool dead;

	Vector3 m_respawnPos;

	int m_collisionsToDie;

	float m_timePassedSinceDeath;
	float m_respawnTimer;
};

