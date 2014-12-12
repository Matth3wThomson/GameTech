#include "Plane.h"


Plane::Plane(const Vector3& normal, float distance, bool normalise){

	type = COLLISION_PLANE;

	if (normalise){
		float length = sqrt(Vector3::Dot(normal, normal));

		this->m_normal = normal / length;
		this->distance = distance / length;
	} else {
		this->m_normal = normal;
		this->distance = distance;
	}
}

bool Plane::SphereInPlane(const Vector3& position, float radius) const {

	float seperation = Vector3::Dot(position, m_normal) + distance; //OLD NOTES

	//TODO: Negative radius?
	if (seperation < -radius) return false; //OLD NOTES

	return true;
}