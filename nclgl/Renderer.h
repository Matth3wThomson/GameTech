#pragma once

#include "OGLRenderer.h"
#include "Camera.h"

class Renderer : public OGLRenderer	{
public:
	Renderer(Window &parent);
	virtual ~Renderer(void);

	virtual void UpdateScene(float msec);
	virtual void RenderScene();

	void ToggleObject();
	void ToggleDepth();
	void ToggleAlphaBlend();
	void ToggleBlendMode();
	void MoveObject(float by);

	void ToggleScissor();
	void ToggleStencil();
	
	void SwitchToPerspective();
	void SwitchToOrthographic();

	void UpdateTextureMatrix(float rotation);
	void ToggleRepeating();
	void ToggleFiltering();

	inline void SetScale(float s){ scale = s; };
	inline void SetRotation(float r){rotation = r; }
	inline void SetPosition(Vector3 p){ position = p; }

protected:

	bool filtering;
	bool repeating;

	float fov;

	float scale;
	float rotation;
	Vector3 position;

	Mesh* triangle;
	Mesh* quad;

	Matrix4 textureMatrix;
	Matrix4 modelMatrix;
	Matrix4 projMatrix;
	Matrix4 viewMatrix;

	bool modifyObject;
	bool usingDepth;
	bool usingAlpha;
	int blendMode;

	bool usingStencil;
	bool usingScissor;

	Camera* camera;
};
