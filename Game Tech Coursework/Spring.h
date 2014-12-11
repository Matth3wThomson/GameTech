#pragma once

#include "PhysicsNode.h"

#define SPRING_VEL_DAMPING 0.9f

class Constraint {
public:
	virtual ~Constraint(void){};
	virtual void Update(float msec) = 0;
};

class Spring : public Constraint {

	friend class Renderer;

public:
	Spring(PhysicsNode* p0, Vector3 localPos0, PhysicsNode* p1, Vector3 localPos1):
		m_lhs(p0), m_rhs(p1), m_localPosL(localPos0), m_localPosR(localPos1){
			/*m_ks = 0.1f;*/
			m_ks = 0.5f;
			m_kd = 0.01f;

			//TODO: This will not work until I fix matrix3 stuff!
			Vector3 pos0 = p0->BuildTransform()*localPos0;
			Vector3 pos1 = p1->BuildTransform()*localPos1;

			m_length = (pos0 - pos1).Length(); //Length of rest
	}
	~Spring(void);

	void Update(float msec); //TODO: Doesnt this need to be virtual?


protected:

	PhysicsNode* m_lhs;
	PhysicsNode* m_rhs;

	Vector3 m_localPosL;
	Vector3 m_localPosR;

	float m_length; //rest length
	float m_ks;		//stiffness
	float m_kd;		//damping

};

