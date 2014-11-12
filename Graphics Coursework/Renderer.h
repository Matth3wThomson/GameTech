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
//TODO: Make shaders matrices multiplied pre-draw function
//TODO: Sort lighting depth issues out for midday
//TODO: Bounding boxes http://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/
//TODO: Store a "camera view and proj" and "light view and proj" to save reinstantiation
//TODO: Make post process only be used if the passes > 0
//TODO: Uninit methods!
//TODO: Create better error intializing methods (return doesnt work within methods!)
//TODO: Sort out how scene nodes draw their bounds, and scale themselves!
//TODO: Sort our shadowing so that it doesn't use the texture matrix!

#define SHADOWSIZE 2048 * 8
#define POST_PASSES 0

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void UpdateScene(float msec);
	void RenderScene();

protected:

	//Generic renderer attributes
	Camera* camera;

	//Generic Shaders
	Shader* passThrough;

	//Functionality properties
	bool pause;
	float movementVar;
	int anim;

	//TEMPORARY!
	float rotation;
	Vector3 scale;
	Vector3 quadPos;

	//Lighting
	Light* light;
	SceneNode* lightSource;

	void DrawLight(const Light* light);
	//End lighting

	//Shadowing
	bool InitShadowBuffers();
	void DeleteShadowBuffers();

	void DrawShadowScene();
	void DrawCombinedScene();

	Shader* sceneShader;
	Shader* shadowShader;

	GLuint shadowTex;
	GLuint shadowFBO;

	//SKYBOX STUFF
	bool InitSkybox();
	void DeleteSkybox();

	void DrawSkybox();

	Shader* skyboxShader;
	GLuint cubeMap;

	//Water additions!
	bool InitWater();
	void DeleteWater();

	void DrawWater();

	Shader* reflectShader;
	GLuint waterTex;
	GLuint waterBump;

	//Scene Objects
	//TODO: Compress objects we no longer need access too!
	bool InitSceneObjects();
	void UpdateSceneObjects(float msec);
	void DeleteSceneObjects();

	Mesh* quad;
	Mesh* sphere;
	MD5FileData* hellData;
	Mesh* heightMap;
	
	SceneNode* root;
	MD5Node* hellNode;
	SceneNode* heightMapNode;
	SceneNode* quadNode;

	Frustum frameFrustum;

	vector<SceneNode*> nodeList;
	vector<SceneNode*> transparentNodes;

	void BuildNodeLists(SceneNode* from, const Vector3& viewPos);
	void SortNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);
	void ClearNodeLists();
	
	//End scene objects

	//TODO: Post Proccess additions
	bool InitPostProcess();
	void DeletePostProcess();

	GLuint bufferFBO;
	GLuint processFBO;

	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;

	GLuint bufferTex;

	Shader* processShader;

	void DrawPostProcess();
	void PresentScene();
	//End of post process additions

	//Text additions
	Font* basicFont;

	void DrawString(const std::string& text, const Vector3& pos, const float size = 10.0f, const bool perspective = false);
	//End text additions

	//Debugging properties
	bool InitDebug();
	void UpdateDebug(float msec);
	void DeleteDebug();

	void DrawDebugOverlay();
	void DrawBounds(SceneNode* node);
	//TODO: Finish this!
	void DrawFrustum();

	static bool debug;

	float frames;
	float fps;
	float timeAcc;

	int objectsDrawn;
	int objectsShadowed;
	bool drawBound;
	//End Debugging Properties

};

