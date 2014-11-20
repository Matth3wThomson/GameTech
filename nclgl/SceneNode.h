#pragma once

#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include <vector>
#include <functional>

class SceneNode
{
public:
	SceneNode(Mesh* m = NULL, Vector4 colour = Vector4(1,1,1,1));
	virtual ~SceneNode(void);

	void SetTransform(const Matrix4& mat){ transform = mat; }
	const Matrix4& GetTransform() const { return transform; }

	void SetPosition(const Vector3& position){ this->position = position; }
	Vector3 GetPosition(){ return position; }

	Matrix4 GetWorldTransform() const { return worldTransform; }

	Vector4 GetColour() const { return colour; }
	void SetColour(Vector4 c){ colour = c; }

	void SetScaleWithParent(bool scale){ scaleWithParent = scale; };
	bool GetScaleWithParent(){ return scaleWithParent; };

	Vector3 GetModelScale() const { return modelScale; }
	void SetModelScale(Vector3 s){ modelScale = s; }
	Vector3 GetWorldScale() const{ return worldScale; };

	float GetRotationAngle(){ return angle; };
	Vector3 GetRotationAxis(){ return rotationAxis; };
	Matrix4 GetRotationMatrix(){ return Matrix4::Rotation(angle, rotationAxis); }

	void SetModelRotation(float angle, const Vector3& axis){ 
		this->rotationAxis = axis;
		this->angle = angle;
	}

	virtual Mesh* GetMesh() const { return mesh; }
	void SetMesh(Mesh* m) { mesh = m; }

	Shader* GetShader() const { return shader; }
	void SetShader(Shader* shader){ this->shader = shader; }

	//NEW
	void SetUpdateShaderFunction(std::function<void()> shaderUpdater){ updateShaderFunction = shaderUpdater; }

	void AddChild(SceneNode* s);
	void RemoveChild(SceneNode* s);

	virtual void Update(float msec);
	virtual void Draw(OGLRenderer& r, const bool useShader = true);

	float GetBoundingRadius() const { return boundingRadius; }
	void SetBoundingRadius(float f){ boundingRadius = f; }

	float GetCameraDistance()const { return distanceFromCamera; }
	void SetCameraDistance(float f){ distanceFromCamera = f; }

	float GetSpecularFactor(){ return specularFactor; };
	void SetSpecularFactor(float sf){ specularFactor = sf; };

	int GetSpecularPower(){ return specularPower; };
	void SetSpecularPower(int sp){ specularPower = sp; };

	static bool CompareByCameraDistance(SceneNode* a, SceneNode* b){
		return (a->distanceFromCamera < b->distanceFromCamera) ? true : false;
	}

	std::vector<SceneNode*>::const_iterator GetChildIteratorStart(){
		return children.begin(); }

	std::vector<SceneNode*>::const_iterator GetChildIteratorEnd(){
		return children.end(); }

protected:
	SceneNode* parent;
	Mesh* mesh;
	Shader* shader;

	std::function<void()> updateShaderFunction;

	bool scaleWithParent;

	Matrix4 worldTransform;
	Matrix4 transform;

	Vector3 position;

	Vector3 worldScale;
	Vector3 modelScale;

	Vector3 rotationAxis;
	float angle;

	float distanceFromCamera;
	float boundingRadius;

	int specularPower;
	float specularFactor;

	Vector4 colour;
	std::vector<SceneNode*> children;
};

