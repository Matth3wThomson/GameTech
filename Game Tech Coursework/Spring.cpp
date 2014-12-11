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
	err /= m_length; //NEW

	//Normalise the direction of the force
	forceDir.Normalise();

	
	Vector3& linVelL = m_lhs->GetLinearVelocity();
	Vector3& linVelR = m_lhs->GetLinearVelocity();

	//Calculate the force to be applied
	Vector3 force = forceDir * (err * m_ks - Vector3::Dot(forceDir, (linVelL, linVelR) * m_kd));

	//Exactly as from the tutorial, the apply force method was never supplied...
	//m_lhs->ApplyForce(posL, force*0.5f);
	//m_rhs->ApplyForce(posR, -force*0.5f);

	//Not sure if this is the correct method that the tutorial wants!
	//m_lhs->ApplyForce(force*0.5f, posL);
	//m_rhs->ApplyForce(-force*0.5f, posR);

	m_lhs->ApplyForce(force*0.5f, Vector3(0,0,0));
	m_rhs->ApplyForce(-force*0.5f, Vector3(0,0,0));

}