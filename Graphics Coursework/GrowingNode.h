#pragma once

#include "../nclgl/SceneNode.h"

#define TOL 0.01

/*
	NOTES:
		-This class was created in order to grow itself based on a given max size,
		 and time to take. It is used by the tree node for its leaves and fruit.
*/
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

