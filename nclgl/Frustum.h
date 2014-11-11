#pragma once

#include "Plane.h"
#include "Matrix4.h"
#include "SceneNode.h"

class Matrix4;

class Frustum
{
public:
	Frustum(void){};
	~Frustum(void){};

	void FromMatrix(const Matrix4& mvp);
	bool InsideFrustum(SceneNode& n);
	
	Plane* Get6Planes(){ return planes; }

protected:
	Plane planes[6];
};

