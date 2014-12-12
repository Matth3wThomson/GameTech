#include "RespawnableEntity.h"


RespawnableEntity::RespawnableEntity(SceneNode* renderNode, PhysicsNode* physicsNode,
									 const Vector3& respawnPos, float respawnTime,
									 int collisionsToDie) : GameEntity(renderNode, physicsNode)
{
	m_respawnPos = respawnPos;
	m_respawnTimer = respawnTime;

	m_collisionsToDie = collisionsToDie;

	m_timePassedSinceDeath = 0.0f;
	dead = false;
}


RespawnableEntity::~RespawnableEntity(void)
{

}

void RespawnableEntity::Update(float msec){
	if (dead){
		if (++m_timePassedSinceDeath > m_respawnTimer){
			physicsNode->SetPosition(m_respawnPos);
			physicsNode->Sleep();
			this->ConnectToSystems();
			dead = false;
			m_timePassedSinceDeath = 0;
		};

	} else {

		if (physicsNode->GetNoCollisions() > m_collisionsToDie){
			this->DisconnectFromSystems();
			physicsNode->ResetCollisionCounter();
			dead = true;
		}
	}

}