#pragma once

#include "GameEntity.h"
#include "Spring.h"
#include "PhysicsSystem.h"
#include "Renderer.h"
#include "GridMesh.h"

//This class represents a peice of cloth in the game world.
//It works by translating the vertices of a grid mesh by a number
//of interconnected physics nodes by springs. The mesh is then rebuffered
//to the GPU
class Cloth : public GameEntity
{
public:
	Cloth(int nodeDensityX, int nodeDensityY, const Vector3& position,
		const Vector3& scale, float sphereRadius, const std::string& filename);
	virtual ~Cloth(void);

	virtual void Update(float msec);

	virtual void ConnectToSystems();
	virtual void DisconnectFromSystems();

protected:
	GridMesh* gridMesh;
	SceneNode* cloth;

	vector<PhysicsNode*> nodes;
	vector<Spring*> springs;
};

