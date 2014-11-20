#include "GrowingNode.h"


GrowingNode::GrowingNode(Vector3& maxScale, float timeToTake, Vector4& colour):
	SceneNode(NULL, colour)
{
	this->maxScale = maxScale;
	this->timeToTake = timeToTake;
	stillGrowing = true;
	timePassed = 0;
}


GrowingNode::~GrowingNode(void)
{
}

void GrowingNode::Update(float msec){

	timePassed += msec;

	if(stillGrowing){

		float multiply = min(1, timePassed/timeToTake);

		this->SetModelScale(maxScale * multiply);

		if ( (1 - multiply) < TOL) stillGrowing = false;

		float greatest = this->GetModelScale().x;

		if (this->GetModelScale().y > greatest) greatest = this->GetModelScale().y;
		if (this->GetModelScale().z > greatest) greatest = this->GetModelScale().z;

		this->SetBoundingRadius(greatest);

	}

	SceneNode::Update(msec);

	//TODO: Also modify bounding radii!
}