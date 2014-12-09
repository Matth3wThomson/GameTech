#include "PhysicsNode.h"

PhysicsNode::PhysicsNode(void)	{
	target = NULL;
	fixed = false;
	m_rest = false;
	m_invMass = 1;
	m_orientation = Quaternion();
	m_narrowPhase = NULL;
	m_broadPhase = NULL;
	lastCollided = NULL;
}

PhysicsNode::PhysicsNode(const Quaternion& orientation, const Vector3& position) {
	m_orientation	= orientation;
	m_position		= position;
	m_invMass = 1;
	fixed = false;
	m_rest = false;
	m_narrowPhase = NULL;
	m_broadPhase = NULL;
	lastCollided = NULL;
}

PhysicsNode::~PhysicsNode(void)	{
	//If these are the same thing!?
	if (m_broadPhase == m_narrowPhase){
		delete m_broadPhase;
	} else {
		delete m_broadPhase;
		delete m_narrowPhase;
	}
	
}

//You will perform your per-object physics integration, here!
//I've added in a bit that will set the transform of the
//graphical representation of this object, too.
void	PhysicsNode::Update(float msec){
	//FUN GOES HERE

	if (!m_rest && !fixed){

		//Need to involve inverse mass here...
		m_linearVelocity += (m_force * m_invMass * msec) * DAMPING_FACTOR;
		m_position += (m_linearVelocity * msec);

		m_angularVelocity += ( (m_invInertia * m_torque) * msec) * DAMPING_FACTOR;
		m_orientation = m_orientation + (m_orientation * (m_angularVelocity * msec * 0.5f));
		m_orientation.Normalise();


		//TODO: Should update collision volumes here if an object gets set to rest...
		// otherwise they could be mistakenly woken up!
		if (abs(m_linearVelocity.x) < REST_TOLERANCE &&
			abs(m_linearVelocity.y) < REST_TOLERANCE &&
			abs(m_linearVelocity.z) < REST_TOLERANCE &&
			abs(m_force.x) < REST_TOLERANCE &&
			abs(m_force.y) < REST_TOLERANCE &&
			abs(m_force.z) < REST_TOLERANCE)
			m_rest = true;
		

	}

	if(target) {
		target->SetTransform(BuildTransform());
		if (m_rest) target->SetColour(Vector4(1,0,0,1));	//DEBUGGING STUFF
		else target->SetColour(Vector4(1,1,1,1));
	}
}

void PhysicsNode::UpdateCollisionPlane(Plane& p){

}

void	PhysicsNode::UpdateCollisionSphere(CollisionSphere& cs){
	cs.m_pos = this->m_position;
}

void PhysicsNode::UpdateCollisionConvex(CollisionConvex& ccv){
	ccv.Update(m_position, m_orientation, m_scale);
}


void PhysicsNode::ApplyForce(const Vector3& force, const Vector3& distanceFromCentre){
	m_force += force;
	m_torque += Vector3::Cross(distanceFromCentre, force);
	m_rest = false;
}

//TODO: Determine if this is correct!
void PhysicsNode::ApplyImpulse(const Vector3& impulse, const Vector3& distanceFromCentre){
	m_linearVelocity += impulse * m_invMass;
	m_angularVelocity += m_invInertia * Vector3::Cross(impulse, distanceFromCentre);
	m_rest = false;
}

/*
This function simply turns the orientation and position
of our physics node into a transformation matrix, suitable
for plugging into our Renderer!

It is cleaner to work with matrices when it comes to rendering,
as it is what shaders expect, and allow us to keep all of our
transforms together in a single construct. But when it comes to
physics processing and 'game-side' logic, it is much neater to
have seperate orientations and positions.

*/
Matrix4		PhysicsNode::BuildTransform() {
	Matrix4 m = m_orientation.ToMatrix();

	m.SetPositionVector(m_position);

	return m;
}

void PhysicsNode::SetInvSphereInertiaMatrix(float mass, float radius){
	float inv_inertia = 1 / ((2.0f * mass * (radius * radius)) * 0.2f);

	/*m_invInertia = Matrix4();
	m_invInertia[0] = inv_inertia;
	m_invInertia[5] = inv_inertia;
	m_invInertia[10] = inv_inertia;
	m_invInertia[15] = 1.0f;*/

	m_invInertia = Matrix3();
	m_invInertia[0] = inv_inertia;
	m_invInertia[4] = inv_inertia;
	m_invInertia[8] = inv_inertia;

}

void PhysicsNode::SetInvCuboidInertiaMatrix(float mass, float height, float width, float length){
	// (One twelfth)
	float inv_inertiaX = 1 / (0.83333333f * mass * ((height * height) + (width * width)));
	float inv_inertiaY = 1 / (0.83333333f * mass * ((length * length) + (width * width)));
	float inv_inertiaZ = 1 / (0.83333333f * mass * ((height * height) + (length * length)));

	//Matrix4 m_invInertia = Matrix4();
	//m_invInertia[0] = inv_inertiaX;
	//m_invInertia[5] = inv_inertiaY;
	//m_invInertia[10] = inv_inertiaZ;
	//m_invInertia[15] = 1.0f; //TODO: What to be done about this?

	m_invInertia = Matrix3();
	m_invInertia[0] = inv_inertiaX;
	m_invInertia[4] = inv_inertiaY;
	m_invInertia[8] = inv_inertiaZ;
}