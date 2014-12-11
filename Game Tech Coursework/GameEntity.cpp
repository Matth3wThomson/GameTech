#include "GameEntity.h"
#include "Renderer.h"
#include "PhysicsSystem.h"

GameEntity::GameEntity(void)	{
	renderNode	= NULL;
	physicsNode = NULL;
}

GameEntity::GameEntity(SceneNode* s, PhysicsNode* p) {
	renderNode	= s;
	physicsNode = p;
}

GameEntity::~GameEntity(void)	{
	DisconnectFromSystems();

	delete renderNode;
	delete physicsNode;
}

void	GameEntity::Update(float msec) {

	for (auto itr = children.begin(); itr != children.end(); ++itr){
		(*itr)->Update(msec);
	}
}

void GameEntity::AddChild(GameEntity* ge){
	children.push_back(ge);
}

//TODO: Implement this
void GameEntity::RemoveChild(GameEntity* ge){
	for (auto itr = children.begin(); itr != children.end(); ++itr){
		if ((*itr) == ge){
			children.erase(itr);
			return;
		}
	}
}

void	GameEntity::ConnectToSystems() {

	if(renderNode && physicsNode) {
		physicsNode->SetTarget(renderNode);
		renderNode->SetTransform(physicsNode->BuildTransform());
	}

	if(renderNode) {
		Renderer::GetRenderer().AddNode(renderNode);
	}

	if(physicsNode) {
		PhysicsSystem::GetPhysicsSystem().AddNode(physicsNode);
	}
}

void	GameEntity::DisconnectFromSystems() {
	if(renderNode) {
		Renderer::GetRenderer().RemoveNode(renderNode);
	}

	if(physicsNode) {
		PhysicsSystem::GetPhysicsSystem().RemoveNode(physicsNode);
	}
}