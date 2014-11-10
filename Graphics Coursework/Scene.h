#pragma once

#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"

class Scene
{
public:
	Scene(void);
	~Scene(void);

	void CreateFrustum(const Matrix4& vp);
	void DrawObjects();
	void DrawSkybox();
	void DrawWater();

protected:
	SceneNode* root;

	vector<SceneNode*> nodes;
	vector<SceneNode*> transparentNodes;

	Frustum frameFrustum;

	Light* light;
};

