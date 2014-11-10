#pragma once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/MD5Mesh.h"
#include "../nclgl/MD5Node.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/TextMesh.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/OBJMesh.h"
#include <algorithm>
#include <sstream>

//TODO: Make sure the far plane is far enough for light frustum culling!
//TODO: Make shaders multiplied pre-draw function
//TODO: Sort lighting depth issues out for midday
//TODO: Bounding boxes http://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/

#define SHADOWSIZE 2048 * 8

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void UpdateScene(float msec);
	void RenderScene();

protected:
	void DrawShadowScene();
	void DrawCombinedScene();

	

	Shader* sceneShader;
	Shader* shadowShader;

	GLuint shadowTex;
	GLuint shadowFBO;

	MD5FileData* hellData;
	MD5Node* hellNode;

	//SKYBOX STUFF
	void DrawSkybox();

	Shader* skyboxShader;
	GLuint cubeMap;

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

	SceneNode* quadNode;
	//End additions

	//TODO: Post Proccess additions
	GLuint bufferFBO;
	GLuint bufferTex;

	GLuint processFBO;

	Shader* processShader;

	//End of post process additions

	//Text additions
	Font* basicFont;

	void DrawString(const std::string& text, const Vector3& pos, const float size = 10.0f, const bool perspective = false);

	Shader* passThrough;

	float frames;
	float fps;
	float timeAcc;

	int objectsDrawn;
	int objectsShadowed;
	//End text additions

	//Heightmap addition
	Mesh* heightMap;
	SceneNode* heightMapNode;
	//End heightmap addition

	bool pause;
	float movementVar;
	int anim;
	bool drawBounds;

	Mesh* quad;

	//BOUNDING 
	Mesh* sphere;
	//end bounding

	Camera* camera;
	Light* light;
};

