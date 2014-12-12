#include "DestructibleSphere.h"

Mesh* DestructibleSphere::sphereMesh = NULL;

DestructibleSphere::DestructibleSphere(int cols, Mesh* sphere, float radius, const Vector3& position,
									   int parts)
{
	m_collisionsToDie = cols;

	if (!sphereMesh)
		sphereMesh = sphere;

	renderNode = new SceneNode(sphereMesh, Vector4(0, 1, 1, 1.0f));
	renderNode->SetShader(Renderer::GetRenderer().phong);

	this->radius = radius;

	physicsNode = new PhysicsNode();
	physicsNode->SetPosition(position);
	physicsNode->SetScale(Vector3(radius, radius, radius));
	physicsNode->SetBroadPhaseVolume(new CollisionSphere(position, radius));
	physicsNode->SetInvSphereInertiaMatrix(100, radius);
	physicsNode->SetMass(100.0f);
	physicsNode->SetConstantAccel(Vector3(0, -0.01f, 0));
	physicsNode->Sleep();

	for (int i=0; i<parts-1; ++i){
		children.push_back(CreateSphereComponent(parts));
	}

	destroyed = false;

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
	pn->SetConstantAccel(physicsNode->GetConstantAccel());

	GameEntity* sphereComponent = new GameEntity(s, pn);
	return sphereComponent;
}

void DestructibleSphere::Update(float msec){

	if (!destroyed){

		if (physicsNode->GetNoCollisions() > m_collisionsToDie){
			this->DisconnectFromSystems();
			destroyed = true;

			Vector3 originalPosition = physicsNode->GetPosition();
			Vector3 scale = physicsNode->GetScale();
			scale.x /= children.size()+1;
			scale.y /= children.size()+1;
			scale.z /= children.size()+1;

			physicsNode->SetScale(scale);

			for (unsigned int i=0; i<children.size(); ++i){
				//Position each child around the radius
				children[i]->GetPhysicsNode().SetPosition(
					originalPosition + 
					Vector3(sin( i / (float) children.size()), 0, cos( i / (float) children.size())));

				//Set their linear velocity according to their new position
				children[i]->GetPhysicsNode().SetLinearVelocity(
					originalPosition - children[i]->GetPhysicsNode().GetPosition());
			}

			for (auto itr = children.begin(); itr != children.end(); ++itr){
				(*itr)->ConnectToSystems();
			}
			this->ConnectToSystems();

			destroyed = true;
		}
	}


	GameEntity::Update(msec);
}
