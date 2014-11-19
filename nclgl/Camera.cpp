#include "Camera.h"

float Camera::timeScaleFactor = 1.0f;

void Camera::UpdateCamera(float msec){
	//Pitch is set to the up and down movement of the y axis
	pitch -= (Window::GetMouse()->GetRelativePosition().y);

	//Yaw is set to the sideways movement (x axis)
	yaw -= (Window::GetMouse()->GetRelativePosition().x);


	//Dont let pitch be less than -90 or greater than 90
	pitch = min(pitch, 90.0f);
	pitch = max(pitch, -90.0f);

	//Similarly with yaw for 360 deg and 0
	if (yaw < 0) yaw += 360.0f;
	if (yaw > 360.0f) yaw -= 360.0f;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_CONTROL))
		timeScaleFactor = 3.0f;
	else if (Window::GetKeyboard()->KeyDown(KEYBOARD_TAB))
		timeScaleFactor = 0.1f;
	else
		timeScaleFactor = 1.0f;

	//FOR FORWARDS/BACKWARDS MOVEMENT
	//We create a rotational matrix that points down the axis of yaw
	//(the way the camera is facing), by rotating about the y axis,
	//and then multiply said matrix by a vector denoting movement
	//* by time since last frame, and add it to our position.
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W))
		position += Matrix4::Rotation(yaw, Vector3(0.0f, 1.0f, 0.0f)) *
		Vector3(0.0f, 0.0f, -1.0f) * msec * timeScaleFactor;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S))
		position -= Matrix4::Rotation(yaw, Vector3(0.0f, 1.0f, 0.0f)) *
		Vector3(0.0f, 0.0f, -1.0f) * msec * timeScaleFactor;

	//Exactly the same as above for left/right movement, other than
	// we move the position in the direction of the x axis, rather than
	// z axis.
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A))
		position += Matrix4::Rotation(yaw, Vector3(0.0f, 1.0f, 0.0f)) *
		Vector3(-1.0f, 0.0f, 0.0f) * msec * timeScaleFactor;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D))
		position -= Matrix4::Rotation(yaw, Vector3(0.0f, 1.0f, 0.0f)) *
		Vector3(-1.0f, 0.0f, 0.0f) * msec * timeScaleFactor;
	

	//For levitation we simply increment (or decrement) the y axis by time.
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT))
		position.y -= msec * timeScaleFactor;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE))
		position.y += msec * timeScaleFactor;
	
}

Matrix4 Camera::BuildViewMatrix(){
	return Matrix4::Rotation(-pitch, Vector3(1,0,0)) *
		Matrix4::Rotation(-yaw, Vector3(0,1,0)) *
		Matrix4::Translation(-position);
}
