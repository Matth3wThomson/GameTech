#include "DestructibleSphere.h"

Mesh* DestructibleSphere::sphereMesh = NULL;

DestructibleSphere::DestructibleSphere(int cols, Mesh* sphere, float radius, const Vector3& position,
									   int parts)
{
	m_collisionsToDie = cols;

	if (!sphereMesh)
		sphereMesh = sphere;

	renderNode = new SceneNode(sphereMesh, Vector4(RAND(), RAND(), RAND(), 1.0f));
	renderNode->SetShader(Renderer::GetRenderer().phong);

	this->radius = radius;

	physicsNode = new PhysicsNode();

	physicsNode->SetPosition(position);

	physicsNode->SetScale(Vector3(radius, radius, radius));
	
	physicsNode->SetBroadPhaseVolume(new CollisionSphere(position, radius));

	physicsNode->SetInvSphereInertiaMatrix(100, radius);

	physicsNode->SetMass(100.0f);
}


DestructibleSphere::~DestructibleSphere(void)
{
}

GameEntity* DestructibleSphere::CreateSphereComponent(int totalParts){
	SceneNode* s = new SceneNode(sphereMesh);
	PhysicsNode* pn = new PhysicsNode();

	s->SetColour(renderNode->GetColour());
	s->SetShader(renderNode->GetShader());

	float subScale = 1 / (float) totalParts;
	
	pn->SetScale(physicsNode->GetScale() * subScale);
	pn->SetBroadPhaseVolume(new CollisionSphere(physicsNode->GetPosition(),
		radius * subScale));
	pn->SetInvSphereInertiaMatrix(physicsNode->GetMass() * subScale, radius);
	pn->SetMass(physicsNode->GetMass() * subScale);

	/*GameEntity* sphereComponent = new GameEntity(*/
	return false;
}

void DestructibleSphere::Update(float msec){
	if (physicsNode->GetNoCollisions() > m_collisionsToDie){
		this->DisconnectFromSystems();
	}

	Vector3 scale = physicsNode->GetScale();
	scale.x /= 2;
	scale.y /= 2;
	scale.z /= 2;

}
