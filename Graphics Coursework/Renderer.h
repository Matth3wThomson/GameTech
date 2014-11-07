#pragma once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/MD5Mesh.h"
#include "../nclgl/MD5Node.h"
#include "../nclgl/Frustum.h"
#include <algorithm>

#define SHADOWSIZE 2048

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void UpdateScene(float msec);
	void RenderScene();

protected:
	void DrawMesh();
	void DrawFloor();
	void DrawShadowScene();
	void DrawCombinedScene();

	Shader* sceneShader;
	Shader* shadowShader;

	GLuint shadowTex;
	GLuint shadowFBO;

	MD5FileData* hellData;
	MD5Node* hellNode;

	//Addition of Scenenode stuff!
	SceneNode* root;
	vector<SceneNode*> nodeList;
	vector<SceneNode*> transparentNodes;

	Frustum frameFrustum;

	void BuildNodeLists(SceneNode* from, const Vector3& viewPos);
	void SortNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);
	void ClearNodeLists();

	SceneNode* floorNode;
	//End additions

	float timePassed;
	int anim;

	Mesh* floor;

	Camera* camera;
	Light* light;
};

