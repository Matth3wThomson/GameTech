#pragma once

#include "GameEntity.h"
#include "Spring.h"
#include "PhysicsSystem.h"
#include "Renderer.h"
#include "GridMesh.h"

class Cloth : public GameEntity
{
public:
	Cloth(int nodeDensityX, int nodeDensityY, const Vector3& position,
		const Vector3& scale, float sphereRadius, const std::string& filename);
	~Cloth(void);

	virtual void Update(float msec);

	virtual void ConnectToSystems();
	virtual void DisconnectFromSystems();

protected:
	GridMesh* gridMesh;
	SceneNode* cloth;

	vector<PhysicsNode*> nodes;
	/*PhysicsNode* nodes[];*/
	vector<Spring*> springs;
};

