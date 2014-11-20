#pragma once

#include "../nclgl/SceneNode.h"

#define TOL 0.01

class GrowingNode : public SceneNode
{
public:
	GrowingNode(Vector3& maxScale, float timeToTake, Vector4& colour);
	~GrowingNode(void);

	virtual void Update(float msec);

protected:

	bool stillGrowing;

	float timePassed;
	float timeToTake;
	Vector3 maxScale;
};

