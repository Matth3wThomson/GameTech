#pragma once

#include "Window.h"
#include "Matrix4.h"
#include "Vector3.h"

class Camera
{
public:
	Camera(void){
		yaw = 0.0f;
		pitch = 0.0f;
	};

	Camera(float pitch, float yaw, Vector3 position){
		this->pitch = pitch;
		this->yaw = yaw;
		this->position = position;
	}

	~Camera(void){};

	void UpdateCamera(float msec = 10.0f);

	Matrix4 BuildViewMatrix();

	inline Vector3 GetPosition() const { return position; }
	inline void SetPosition(Vector3 val){ position = val; }

	inline float GetYaw() const { return yaw; }
	inline void SetYaw(float y){ yaw = y; }

	inline float GetPitch() const { return pitch; }
	inline void SetPitch(float p){ pitch = p; }

protected:
	float yaw;	//Camera's heading
	float pitch; //Degrees up or down camera is facing

	static float timeScaleFactor;

	Vector3 position;
};

