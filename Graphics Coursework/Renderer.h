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
#include "ParticleEmitterNode.h"
#include "TreeNode.h"
#include <algorithm>
#include <sstream>

//TODO: Make sure the far plane is far enough for light frustum culling!
//TODO: Make shaders matrices multiplied pre-draw function
//TODO: Bounding boxes http://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/
//TODO: Store a "camera view and proj" and "light view and proj" to save reinstantiation
//TODO: Make post process only be used if the passes > 0
//TODO: GL CULL FACE NEEDS TO BE CALLED ONLY IN DRAW FUNCTIONS!
//TODO: Combine light node and source to become one!?
//TODO: Get textures and bump maps back onto hell knight
//TODO: Make everything possible scene nodes so frustum culling makes sense!?
//TODO: Sort out the open GL issues found and hence why nvidia nsight isnt working,
	//and remove all the debugging code!!
//TODO: Have the map overlay inserted into the final scene!


#define SHADOWSIZE 2048 * 8
#define BLUR_PASSES 10

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
	Matrix4 cameraProjMat;
	Matrix4 ortho;

	//Generic Shaders
	Shader* passThrough;

	void UpdateShadersPerFrame();

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

	void UpdateCombineSceneShaderMatricesPF();
	void UpdateCombineSceneShaderMatricesPO();
	void DrawShadowScene();
	void DrawCombinedScene();

	Shader* sceneShader;
	Shader* shadowShader;

	GLuint shadowTex;
	GLuint shadowFBO;

	Matrix4 shadowVPMatrix;

	//SKYBOX STUFF
	bool InitSkybox();
	void DeleteSkybox();

	void DrawSkybox();

	Shader* skyboxShader;
	GLuint cubeMap;

	//Water additions!
	bool InitWater();
	void DeleteWater();
	void UpdateWaterShaderMatricesPO();
	void UpdateWaterShaderMatricesPF();

	//void DrawWater(bool shadowMap = false);

	Shader* reflectShader;
	GLuint waterTex;
	GLuint waterBump;
	SceneNode* waterNode;
	Mesh* waterQuad;

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

	Frustum frameFrustum;

	vector<SceneNode*> nodeList;
	vector<SceneNode*> transparentNodes;

	void BuildNodeLists(SceneNode* from, const Vector3& viewPos);
	void SortNodeLists();
	void ClearNodeLists();
	
	//End scene objects

	//TODO: Post Proccess additions
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
	Shader* sobelShader;
	Shader* sobelDepthShader;
	Shader* doubVisShader;
	Shader* bloomShader;
	Shader* antiAliasShader;

	bool sobel;
	bool sobelDepth;
	bool antiAlias;
	bool blur;
	bool dubVis;
	bool bloom;

	void UpdatePostProcess(float msec);
	void DrawPostProcess();
	void SetupPPMatrices();

	void Bloom();
	void Sobel();
	void SobelDepth();
	void SobelAlias();
	void DoubleVision();
	void Blur();

	void PresentScene();
	//End of post process additions

	//Attempt custom bloom!
	/*Shader* renderColour;
	Shader* bloomShader;

	GLuint bloomTex;*/

	//End attempts of custom bloom!

	//PARTICLE EMISSION STUFF
	Shader* particleShader;
	ParticleEmitter* rain;
	ParticleEmitter* snow;

	ParticleEmitterNode* rainNode;
	ParticleEmitterNode* snowNode;

	void UpdateParticleShaderMatricesPO();

	bool raining;
	bool snowing;

	bool InitParticles();
	void DeleteParticles();

	void UpdateParticles(float msec);
	//End attempt particle emission

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

	//PLANT STUFF!?
	Mesh* cylinder;

	void DrawCylinder();
	//End plant stuff 

};

