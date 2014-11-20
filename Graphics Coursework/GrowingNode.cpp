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

	//We dont continuially want to be doing this calculation once we are
	//at full size!
	if(stillGrowing){

		float multiply = min(1, timePassed/timeToTake);

		this->SetModelScale(maxScale * multiply);

		if ( (1 - multiply) < TOL) stillGrowing = false;

		float greatest = this->GetModelScale().x;

		if (this->GetModelScale().y > greatest) greatest = this->GetModelScale().y;
		if (this->GetModelScale().z > greatest) greatest = this->GetModelScale().z;

		//Set the bounding radius to be whatever has the greatest axis on the scale matrix!
		this->SetBoundingRadius(greatest);

	}

	//Do the rest of our updating as per usual.
	SceneNode::Update(msec);
}