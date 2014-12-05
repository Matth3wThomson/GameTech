#pragma once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/MD5Mesh.h"
#include "../nclgl/MD5Node.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/TextMesh.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/OBJMesh.h"
#include "../nclgl/ParticleEmitter.h"
#include "PhysicsSystem.h"

#include "ParticleEmitterNode.h"
#include "TreeNode.h"
#include <algorithm>
#include <sstream>


#define SHADOWSIZE 2048 * 8
#define BLUR_PASSES 5
#define FONT_SIZE 16.0f


/**
	NOTES:
		-In an effort to make this class more tidy I have put all methods relevant to
		 elements in the scene into their own CPP file where necessary. I have stuck to a
		 particular format of Init -> Update -> Draw -> Delete for each section where
		 relevant to ease readability.
		-To maintain this cleanliness, all methods comments will be written in the cpp file.
		-Because methods started to get fairly long winded names, there are a few acronyms used:
			-PF -> Per frame
			-PO -> Per object
			-HM -> heightMap
*/
class Renderer : public OGLRenderer
{
public:
	virtual void UpdateScene(float msec);
	virtual void RenderScene();

	static bool Initialise(){
		instance = new Renderer(Window::GetWindow());
		return instance->HasInitialised();
	}

	static void Destroy(){
		delete instance;
	}

	void SetCamera(Camera* c){ camera = c; }

	void AddNode(SceneNode* n){ root->AddChild(n); };
	void RemoveNode(SceneNode* n){ root->RemoveChild(n); };

	static Renderer& GetRenderer(){ return *instance; }

protected:

	Renderer(Window& parent);
	virtual ~Renderer();

	static Renderer* instance;

	//Generic renderer attributes
	Camera* camera;
	Matrix4 cameraProjMat;
	Matrix4 ortho;

	//Generic Shaders
	Shader* passThrough;

	Shader* phong;

	void UpdateGenericShadersPF();

	void UpdateShadersPerFrame();

	//Functionality properties
	bool pause;
	float timeOfDay;
	int anim;

	bool timeSlowed;
	bool dayTimeSpeedIncrease;

	//Lighting
	Light* light;
	SceneNode* lightSource;

	//Heightmap multitexturing
	void UpdateHeightMapShaderPO();
	void UpdateHeightMapShaderPF();

	Shader* heightMapShader;

	HeightMap* heightMap;
	GLuint heightMapTex;
	GLuint HMToonTex;

	GLuint heightMapHighTex;
	GLuint HMToonHighTex;

	//Skybox
	bool InitSkybox();
	void DeleteSkybox();

	void UpdateSkybox(float msec);
	void DrawSkybox();

	Shader* skyboxShader;
	GLuint cubeMap;

	float skyboxLight;

	//Water
	bool InitWater();
	void DeleteWater();

	void UpdateWater(float msec);
	void UpdateWaterShaderMatricesPO();
	void UpdateWaterShaderMatricesPF();

	Shader* reflectShader;
	SceneNode* waterNode;
	Mesh* waterQuad;
	float rotation;

	//Scene Objects
	bool InitSceneObjects();
	void UpdateSceneObjects(float msec);
	void DeleteSceneObjects();


	Mesh* quad;
	Mesh* sphere;
	MD5FileData* hellData;

	SceneNode* root;
	MD5Node* hellNode;
	SceneNode* heightMapNode;

	Frustum frameFrustum;

	vector<SceneNode*> nodeList;
	vector<SceneNode*> transparentNodes;

	void BuildNodeLists(SceneNode* from, const Vector3& viewPos, bool transparents = true);
	void SortNodeLists();
	void ClearNodeLists();

	//POST PROCESS
	bool InitPostProcess();
	void DeletePostProcess();
	void UpdatePPShaderMatrices();

	GLuint bufferFBO;
	GLuint processFBO;

	//We have multiple buffers for ping/pong!
	unsigned int toDrawTo; //Use this to keep track of the texture to be used next
	const static unsigned int PPtextures = 2; //The number of Post process textures
	inline unsigned int GetDrawTarget(){ return bufferColourTex[toDrawTo]; }; //Gets the array location of the next texture to draw to
	inline unsigned int GetLastDrawn(){
		return ((((int) toDrawTo)-1) < 0)? bufferColourTex[PPtextures-1] : bufferColourTex[toDrawTo-1];	}; //Gets the array location of the last texture drawn to
	inline void PPDrawn(){ ++toDrawTo %= PPtextures; }; //Signifies that a Post pass has been done.

	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;

	GLuint bufferTex;

	Shader* blurShader;
	Shader* doubVisShader;
	Shader* sobelShader;
	Shader* sobelDepthShader;
	Shader* quantizeColShader;
	Shader* fogShader;
	Shader* bloomShader;

	bool sobel;
	bool sobelDepth;
	bool quantizeCol;
	bool fog;
	bool blur;
	bool dubVis;
	bool bloom;

	void UpdatePostProcess(float msec);
	void DrawPostProcess();
	void SetupPPMatrices();

	void Bloom();
	void Sobel();
	void SobelDepth();
	void QuantizeCol();
	void Fog();
	void DoubleVision();
	void Blur();

	void PresentScene();

	//PARTICLE EMISSION
	Shader* particleShader;
	ParticleEmitter* rain;
	ParticleEmitter* snow;

	ParticleEmitterNode* rainNode;
	ParticleEmitterNode* snowNode;

	bool raining;
	bool snowing;

	bool InitParticles();
	void DeleteParticles();

	void UpdateParticles(float msec);

	//Text additions
	Font* basicFont;

	void DrawString(const std::string& text, const Vector3& pos, const float size = 10.0f, const bool perspective = false);

	//Debugging properties
	bool InitDebug();
	void UpdateDebug();
	void DeleteDebug();

	void DrawDebugOverlay();
	void DrawBounds(SceneNode* node);

	static bool debug;

	GameTimer gt;

	float frames;
	float fps;
	float timeAcc;

	int objectsDrawn;
	int objectsShadowed;
	bool drawBound;
	
	//Shadowing
	bool InitShadowBuffers();
	void DeleteShadowBuffers();

	void UpdateCombineSceneShaderMatricesPF();
	void UpdateCombineSceneShaderMatricesPO();
	void DrawShadowScene();
	void DrawCombinedScene();

	Shader* sceneShader;
	Shader* shadowShader;

	GLuint shadowTex;
	GLuint shadowFBO;

	Matrix4 shadowVPMatrix;

	//DEFERRED RENDERING
	bool InitDeferredRendering();
	void DeleteDeferredRendering();

	void UpdateDeferredRendering(float msec);
	void DrawDeferredRendering();

	GLuint fireParticleTex;

	void GenerateScreenTexture(GLuint& into, bool depth = false);

	void FillBuffers();
	void DrawPointLights();
	void CombineBuffers();

	Light* pointLights;
	int numPointLights;
	Mesh* icoSphere;

	Shader* pointLightShader;
	Shader* combineShader;

	GLuint pointLightFBO;

	GLuint originalSceneTex;
	GLuint bufferNormalTex;
	GLuint lightEmissiveTex;
	GLuint lightSpecularTex;
};

