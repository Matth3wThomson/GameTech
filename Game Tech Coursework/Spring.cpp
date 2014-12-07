#include "Spring.h"



Spring::~Spring(void)
{
}

void Spring::Update(float msec){
	Vector3 posL = m_lhs->BuildTransform() * m_localPosL;
	Vector3 posR = m_rhs->BuildTransform() * m_localPosR;

	//Work out the direction between the two nodes
	Vector3 dir = posR - posL;

	//Calculate a value for the length between the two points - rest length
	float err = dir.Length() - m_length;

	dir.Normalise();

	Vector3 linVelL = m_lhs->GetLinearVelocity();
	Vector3 linVelR = m_lhs->GetLinearVelocity();

	Vector3 force = dir * (err * m_ks - Vector3::Dot(dir, (linVelL, linVelR) * m_kd));

	//Exactly as from the tutorial, the apply force method was never supplied...
	//m_lhs->ApplyForce(posL, force*0.5f);
	//m_rhs->ApplyForce(posR, -force*0.5f);

	m_lhs->ApplyForce(force*0.5f, posL);
	m_rhs->ApplyForce(force*0.5f, posR);

}