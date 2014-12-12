#include "Spring.h"



Spring::~Spring(void)
{
}

void Spring::Update(float msec){
	//F = -kx - c (n . vab)

	//Calculate the world positions for the local positions
	Vector3 posL = m_lhs->BuildTransform() * m_localPosL;
	Vector3 posR = m_rhs->BuildTransform() * m_localPosR;

	//Work out the direction between the two nodes
	Vector3 forceDir = posR - posL;

	//Calculate a value for the length between the two points - rest length.
	//This is basically the amount of extension the spring has undertaken. (x)
	float err = forceDir.Length() - m_length;

	//We divide it by the rest length of the spring to get a normalized value for the 
	// extension of the spring
	err /= m_length;

	//Normalise the direction of the force
	forceDir.Normalise();

	Vector3& linVelL = m_lhs->GetLinearVelocity();
	Vector3& linVelR = m_rhs->GetLinearVelocity();

	//Calculate the force to be applied
	Vector3 force = forceDir * (err * m_ks - Vector3::Dot(forceDir, (linVelL - linVelR) * m_kd));

	m_lhs->ApplyForce(force*0.5f, Vector3(0,0,0));
	m_rhs->ApplyForce(-force*0.5f, Vector3(0,0,0));

}