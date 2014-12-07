#include "PhysicsSystem.h"

PhysicsSystem* PhysicsSystem::instance = 0;
float PhysicsSystem::timestep = 1.0f/120.0f;

PhysicsSystem::PhysicsSystem(void) : octTree(OctTree(1000, 3, 2)){
	collisionCount = 0;
	physTimer = GameTimer();
	timePassed = 0;
	updateRate = 0;
	physFrames = 0;
	timePassed = 0.0f;

	//Requires world size, max number of objects per node, and max depth of tree
	//broadPhase = OctTree(Vector3(4000,4000,4000), 6, 2);
}

PhysicsSystem::~PhysicsSystem(void)	{

}

void	PhysicsSystem::Update(float msec){

	timestep = msec;

	++physFrames;
	timePassed += physTimer.GetTimedMS();
	if (timePassed > 1000.0f){ //After each second
		updateRate = physFrames;	//Update the physics update rate 
		physFrames = 0;				//And reset our variables to count it
		timePassed -= 1000.0f;
	}

	BroadPhase();
	NarrowPhase();

	//ResolveCollisions(); //Resolve after all overlaps have been corrected and intersections found
	std::lock_guard<std::mutex> lock(nodesMutex);
	for(vector<PhysicsNode*>::iterator i = allNodes.begin(); i != allNodes.end(); ++i) {
		(*i)->Update(msec);
	}
}

void	PhysicsSystem::BroadPhase() {
	//Sort all objects into octree based on some sort of collision properties?
	std::lock_guard<std::mutex> lock(nodesMutex);
	octTree.Update();
}


//HOW do we use an enum to denote collision volume type and call the correct functions
//when the pointers are still of the incorrect type? Dynamic cast post enum check?

//TODO: Because iterators get invalidated when an element is added to a vector elsewhere,
//		we need concurrency protection on our vector of physics nodes.

//TODO: Because when 3 entities overlap, how do we resolve correctly? Translate out of collision
//		as soon as it is found is wrong? Doing collision resolution for an object twice will break it?
//void	PhysicsSystem::NarrowPhase(){
//
//	//Then perform collision detection on each section of the octree recursively
//	std::lock_guard<mutex> lock(nodesMutex);
//
//	for (vector<PhysicsNode*>::iterator i = allNodes.begin(); i != allNodes.end(); ++i){
//		for (auto j = i+1; j != allNodes.end(); ++j){
//
//			CollisionVolume* cv1 = (*i)->GetNarrowPhaseVolume();
//			CollisionVolume* cv2 = (*j)->GetNarrowPhaseVolume();
//
//			if (cv1 && cv2){
//
//				CollisionVolumeType cvt1 = cv1->GetType();
//				CollisionVolumeType cvt2 = cv2->GetType();
//				CollisionData cd;
//
//				switch (cvt1){
//				case COLLISION_PLANE:
//					if (cvt2 == COLLISION_SPHERE){
//						Plane* p = dynamic_cast<Plane*>(cv1);
//						CollisionSphere* cs = dynamic_cast<CollisionSphere*>(cv2);
//
//						//We must update our collision volumes
//						//UpdateCollisionSphere(**j, *cs);
//						(*j)->UpdateCollisionSphere(*cs);
//						(*i)->UpdateCollisionPlane(*p);
//
//						//Sphere in plane is the wrong way around...
//						/*if (p->SphereInPlane(cs->m_pos, cs->m_radius, &cd)){*/
//						if (SphereInColPlane(*p, cs->m_pos, cs->m_radius, &cd)){
//							//Wrong side of the plane... but what is the limit to the plane?
//							std::cout << "SP COLLISION\n";
//							AddCollisionImpulse(*(*j), *(*i), cd);
//							++collisionCount;
//							 
//						}
//					};
//
//					if (cvt2 == COLLISION_AABB){
//
//					}; 
//
//					break;
//				case COLLISION_SPHERE:
//					if (cvt2 == COLLISION_PLANE) break;
//					if (cvt2 == COLLISION_SPHERE){
//						CollisionSphere* cs1 = dynamic_cast<CollisionSphere*>(cv1);
//						CollisionSphere* cs2 = dynamic_cast<CollisionSphere*>(cv2);
//
//						(*i)->UpdateCollisionSphere(*cs1);
//						(*j)->UpdateCollisionSphere(*cs2);
//
//						if (SphereSphereCollision(*cs1, *cs2, &cd)){
//							std::cout << "SS COLLISION\n";
//							AddCollisionImpulse(*(*i), *(*j), cd);
//							++collisionCount;
//						}
//					};
//					if (cvt2 == COLLISION_AABB); 
//
//					break;
//				case COLLISION_AABB:
//					if (cvt2 == COLLISION_PLANE) break;
//					if (cvt2 == COLLISION_SPHERE);
//					if (cvt2 == COLLISION_AABB); 
//
//					break;
//				}
//			}
//		}
//	}
//}

void	PhysicsSystem::NarrowPhase(){

	//Then perform collision detection on each section of the octree recursively
	std::lock_guard<mutex> lock(nodesMutex);
	
	NarrowPhaseTree(octTree.root);
}

void PhysicsSystem::NarrowPhaseTree(OctNode& on){
	if (on.octNodes.size() != 0){
		for (auto itr = on.octNodes.begin(); itr != on.octNodes.end(); itr++){
			NarrowPhaseTree(**itr);
		}
	} else {
		NarrowPhaseVector(on.physicsNodes);
	}
}

void PhysicsSystem::NarrowPhaseVector(std::vector<PhysicsNode*>& np){

	for (vector<PhysicsNode*>::iterator i = np.begin(); i != np.end(); ++i){
		for (auto j = i+1; j != np.end(); ++j){

			CollisionVolume* cv1 = (*i)->GetNarrowPhaseVolume();
			CollisionVolume* cv2 = (*j)->GetNarrowPhaseVolume();

			if (cv1 && cv2){

				CollisionVolumeType cvt1 = cv1->GetType();
				CollisionVolumeType cvt2 = cv2->GetType();
				CollisionData cd;

				switch (cvt1){
				case COLLISION_PLANE:
					if (cvt2 == COLLISION_SPHERE){
						Plane* p = dynamic_cast<Plane*>(cv1);
						CollisionSphere* cs = dynamic_cast<CollisionSphere*>(cv2);

						//We must update our collision volumes
						(*j)->UpdateCollisionSphere(*cs);
						(*i)->UpdateCollisionPlane(*p);

						//Sphere in plane is the wrong way around...
						/*if (p->SphereInPlane(cs->m_pos, cs->m_radius, &cd)){*/
						if (SphereInColPlane(*p, cs->m_pos, cs->m_radius, &cd)){
							//Wrong side of the plane... but what is the limit to the plane?
							std::cout << "SP COLLISION\n";
							AddCollisionImpulse(*(*j), *(*i), cd);
							++collisionCount;
							 
						}
					};

					if (cvt2 == COLLISION_AABB){

					}; 

					break;
				case COLLISION_SPHERE:
					if (cvt2 == COLLISION_PLANE) break;
					if (cvt2 == COLLISION_SPHERE){
						CollisionSphere* cs1 = dynamic_cast<CollisionSphere*>(cv1);
						CollisionSphere* cs2 = dynamic_cast<CollisionSphere*>(cv2);

						(*i)->UpdateCollisionSphere(*cs1);
						(*j)->UpdateCollisionSphere(*cs2);

						if (SphereSphereCollision(*cs1, *cs2, &cd)){
							std::cout << "SS COLLISION\n";
							AddCollisionImpulse(*(*i), *(*j), cd);
							++collisionCount;
						}
					};
					if (cvt2 == COLLISION_AABB); 

					break;
				case COLLISION_AABB:
					if (cvt2 == COLLISION_PLANE) break;
					if (cvt2 == COLLISION_SPHERE);
					if (cvt2 == COLLISION_AABB); 

					break;
				}
			}
		}
	}
}



void PhysicsSystem::ResolveCollisions(){

}

void	PhysicsSystem::AddNode(PhysicsNode* n){
	std::lock_guard<mutex> lock(nodesMutex);
	allNodes.push_back(n);
 	octTree.AddPhysicsNode(n);
}

void	PhysicsSystem::RemoveNode(PhysicsNode* n) {
	for(vector<PhysicsNode*>::iterator i = allNodes.begin(); i != allNodes.end(); ++i) {
		if((*i) == n) {
			allNodes.erase(i);
			return;
		}
	}
}

bool PhysicsSystem::LineLineIntersect(const Line& l1, const Line& l2, float* t1, float* t2) const {
	const Vector3& p0 = l1.m_pos1;
	const Vector3& p1 = l1.m_pos2;
	const Vector3& p2 = l2.m_pos1;
	const Vector3& p3 = l2.m_pos2;

	const float div = (p3.y - p2.y) * (p1.x - p0.x)
		- (p3.x - p2.x) * (p1.y - p0.y);

	//Lines are parallel
	if (abs(div) < 0.000001f) return false;

	const float ta = ( (p3.x - p2.x) * (p0.y - p2.y)
		- (p3.y - p2.y) * (p0.x - p2.x) ) / div;

	if (ta < 0 || ta > 1.0f) return false;

	const float tb = ( ( p1.x - p0.x) * (p0.y - p2.y)
		- (p1.y - p0.y) * (p0.x - p2.x) ) / div;

	if (tb < 0 || tb > 1.0f) return false;

	if (t1) (*t1)=ta;
	if (t2) (*t2)=tb;

	return true;
}

//Physics Correct plane collision
bool PhysicsSystem::SphereInColPlane(const Plane& p, const Vector3& pos, float rad, CollisionData* cd) const {
	//TODO: Added abs
	float seperation = Vector3::Dot(pos, p.m_normal) - p.distance;

	//if (Vector3::Dot(pos, p.m_normal) < 0) return false;

	//TODO: Remember, this function will only return true if the sphere and plane intersect
	if (!(abs(seperation) < rad)) return false;

	if (cd){
		cd->m_penetration = rad - seperation;
		cd->m_normal = p.m_normal; //TODO: INVERSE?
		cd->m_point = pos - (p.m_normal * seperation);
	}

	return true;
}

bool PhysicsSystem::SphereSphereCollision(const CollisionSphere &s0, const CollisionSphere &s1, CollisionData *collisionData) const {
	const float distSq = ( s0.m_pos - s1.m_pos ).LengthSq();

	const float sumRadius = (s0.m_radius + s1.m_radius);
	//assert(distSq > 0.00001f );

	if (distSq < sumRadius * sumRadius){
		if (collisionData){
			collisionData->m_penetration = sumRadius - sqrtf( distSq );
			collisionData->m_normal	= (s0.m_pos - s1.m_pos).Normalise();
			collisionData->m_point = s0.m_pos - collisionData->m_normal
				* (s0.m_radius - collisionData->m_penetration * 0.5f);
		}
		return true;
	}

	return false;
}

//TODO: implement 
bool PhysicsSystem::SphereAABBCollision(const CollisionSphere& sphere, const CollisionAABB& cube, CollisionData* collisionData) const {
	return false;
}

bool PhysicsSystem::AABBCollision(const CollisionAABB &cube0, const CollisionAABB &cube1) const {

	//Check X Axis
	float dist = cube0.m_position.x - cube1.m_position.x;
	float sum = (cube0.m_halfSize.x + cube1.m_halfSize.x);

	if (dist < sum){

		//Check Y Axis
		dist = cube0.m_position.y - cube1.m_position.y;
		sum = (cube0.m_halfSize.y + cube1.m_halfSize.y);

		if (dist < sum){

			//Check Z Axis
			dist = cube0.m_position.z - cube1.m_position.z;
			sum = (cube0.m_halfSize.z + cube1.m_halfSize.z);

			//Overlapped shapes
			if (dist < sum) return true;
		}

	}

	return false;
}

bool PhysicsSystem::PointInConvexPolygon(const Vector3 testPosition, Vector3 * convexShapePoints, int numPoints) const {

	for (int i=0; i<numPoints; ++i){
		const int i0 = i;
		const int i1 = (i+1) % numPoints;

		const Vector3& p0 = convexShapePoints[i0];
		const Vector3& p1 = convexShapePoints[i1];

		//We need two things for each edge, a point on the edge and the normal
		const Vector3 norm = Vector3::Cross(Vector3(0,0,1), (p0-p1).Normalise());

		//Use the plane equation to calculate d, and to determine if our point is on
		//the positive or negative side of the plane
		const float d = Vector3::Dot(norm, p0);

		//Calculate which side our test point is on
		//INSIDE: +ve. OUTSIDE: -ve ON PLANE: zero
		const float s = d - Vector3::Dot( norm, testPosition );

		if (s < 0.0f) return false;

	}

	return true;
}

bool PhysicsSystem::PointInConcavePolygon( const Vector3* shapePoints, const int numPoints, const Vector3& testPoint) const {

	int intersectionCount = 0;

	for (int i=0; i<numPoints; ++i){
		const int i0 = i;
		const int i1 = (i+1)%numPoints;

		const Vector3& p0 = shapePoints[i0];
		const Vector3& p1 = shapePoints[i1];

		bool intersect = LineLineIntersect( Line(p0, p1),
			Line(testPoint, testPoint + Vector3(1000,1000,0)) );

		if (intersect) intersectionCount++;
	}

	if (intersectionCount % 2 == 0) return false;

	return true;
}

//void PhysicsSystem::UpdateCollisionSphere(const PhysicsNode& pn, CollisionSphere& cs){
//	cs.m_pos = pn.GetPosition();
//}

//void PhysicsSystem::UpdateCollisionPlane(const PhysicsNode& pn, Plane& p){
//
//}
//
//void PhysicsSystem::UpdateCollisionAABB(const PhysicsNode& pn, CollisionAABB& aabb){
//
//}

//Consider replacing the final 3 parameters with collision data?
//void PhysicsSystem::AddCollisionImpulse( PhysicsNode& pn0, PhysicsNode& pn1,
//								const Vector3& hitPoint, const Vector3& normal,
//								float penetration)
//{
//	//If the object weighs more than 1000kg then set it immovable
//	float invMass0 = (pn0.m_invMass < 0.001f) ? 0.0f : pn0.m_invMass;
//	float invMass1 = (pn1.m_invMass < 0.001f) ? 0.0f : pn1.m_invMass;
//
//	//If the object is fixed in place do the same
//	invMass0 = (pn0.fixed)? 0.0f : invMass0;
//	invMass1 = (pn1.fixed)? 0.0f : invMass1;
//
//	//TODO: Since these are const, why not make them references?
//	const Matrix4 worldInvInertia0 = pn0.m_invInertia;
//	const Matrix4 worldInvInertia1 = pn1.m_invInertia;
//
//	//Both immovable, dont continue
//	if ( (invMass0+invMass1) == 0.0 ) return;
//
//	//Calculate the vector of the point of contact to the center of the shape.
//	// (The contact normal for each shape?)
//	Vector3 r0 = hitPoint - pn0.m_position;
//	Vector3 r1 = hitPoint - pn1.m_position;
//
//	Vector3 v0 = pn0.m_linearVelocity + Vector3::Cross(pn0.m_angularVelocity, r0);
//	Vector3 v1 = pn1.m_linearVelocity + Vector3::Cross(pn1.m_angularVelocity, r1);
//
//	//Relative velocity
//	Vector3 dv = v0 - v1;
//
//	float relativeMoment = -Vector3::Dot(dv, normal);
//	
//	//If the objects are moving away from each other then there isnt a need to apply an impulse!
//	if (relativeMoment < -0.01f) return;
//
//	//Normal Impulse
//	{ //TODO: Extra scope?
//		float e = 0.0f;
//
//		float normDiv = Vector3::Dot(normal, normal)
//			* ( (invMass0 + invMass1)
//			+ Vector3::Dot(normal,
//			worldInvInertia0 * Vector3::Cross( Vector3::Cross( r0, normal), r0) 
//			+ Vector3::Cross( worldInvInertia1 * Vector3::Cross(r1, normal),r1) ) );
//
//		float jn = -1 * (1+e) * Vector3::Dot(dv, normal) /normDiv;
//
//		jn = jn + (penetration * 1.5f);
//
//		pn0.m_linearVelocity += (normal * invMass0) * jn;
//		pn0.m_angularVelocity += worldInvInertia0 * Vector3::Cross(r0, normal * jn);
//
//		pn1.m_linearVelocity -= (normal * invMass1) * jn;
//		pn1.m_angularVelocity -= worldInvInertia1 * Vector3::Cross(r1, normal * jn);
//	}
//
//	//TANGENT IMPULSE IS THIS NECESSARY?
//	{
//		Vector3 tangent = Vector3(0,0,0);
//		tangent = dv - (normal * Vector3::Dot(dv, normal));
//		tangent.Normalise();
//
//		float tangDiv = invMass0 + invMass1
//			+ Vector3::Dot( tangent,
//			Vector3::Cross(pn0.m_invInertia * (Vector3::Cross(r0, tangent) ), r0)
//			+ Vector3::Cross(pn1.m_invInertia * (Vector3::Cross(r1, tangent) ), r1) );
//
//		float jt = -1 * Vector3::Dot(dv, tangent) / tangDiv;
//
//		//TODO: Clamping here?
//
//		//Apply contact impulse
//		pn0.m_linearVelocity += tangent * jt * invMass0;
//		pn0.m_angularVelocity += worldInvInertia0 * Vector3::Cross(r0, tangent * jt);
//
//		pn1.m_linearVelocity -= tangent * jt * invMass1;
//		pn1.m_angularVelocity -= worldInvInertia1 * Vector3::Cross(r1, tangent * jt);
//
//	}
//}

void PhysicsSystem::AddCollisionImpulse( PhysicsNode& pn0, PhysicsNode& pn1,
								const CollisionData& cd)
{
	pn0.m_rest = false;
	pn1.m_rest = false;

	//If the object weighs more than 1000kg then set it immovable
	float invMass0 = (pn0.m_invMass < 0.001f) ? 0.0f : pn0.m_invMass;
	float invMass1 = (pn1.m_invMass < 0.001f) ? 0.0f : pn1.m_invMass;

	//If the object is fixed in place do the same
	invMass0 = (pn0.fixed)? 0.0f : invMass0;
	invMass1 = (pn1.fixed)? 0.0f : invMass1;

	//TODO: Since these are const, why not make them references?
	const Matrix4 worldInvInertia0 = pn0.m_invInertia;
	const Matrix4 worldInvInertia1 = pn1.m_invInertia;

	//Both immovable, dont continue
	if ( (invMass0+invMass1) == 0.0f ) return;

	//Calculate the vector of the point of contact to the center of the shape.
	// (The contact normal for each shape?)
	Vector3 r0 = cd.m_point - pn0.m_position;
	Vector3 r1 = cd.m_point - pn1.m_position;

	//Now move them based on a projection method involving relative mass of each object,
	//so that objects with a ridiculous mass dont get projected out of collision with very light
	//objects
	float totalInvMass = (invMass0 + invMass1);

	//Move each object in the direction of the normal, based on their relative weighting
	pn0.m_position += cd.m_normal * cd.m_penetration * (invMass0 / totalInvMass);
	//pn0.m_position += cd.m_normal * 0.00001f;

	pn1.m_position -= cd.m_normal * cd.m_penetration * (invMass1 / totalInvMass);
	//pn1.m_position -= cd.m_normal * 0.00001f;

	Vector3 v0 = pn0.m_linearVelocity + Vector3::Cross(pn0.m_angularVelocity, r0);
	Vector3 v1 = pn1.m_linearVelocity + Vector3::Cross(pn1.m_angularVelocity, r1);

	//Relative velocity
	Vector3 dv = v0 - v1;

	//Swap this sign?
	float relativeMovent = -Vector3::Dot(dv, cd.m_normal);
	
	//If the objects are moving away from each other then there isnt a need to apply an impulse!
	if (relativeMovent < -0.01f) return;

	//Normal Impulse
	{ //TODO: Extra scope?
		float e = 0.9f; //Elasticity... Needs to be moved to individual physics nodes?
		
		/*float normDiv = Vector3::Dot(cd.m_normal, cd.m_normal)
			* ( (invMass0 + invMass1)
			+ Vector3::Dot(cd.m_normal,
			Vector3::Cross( worldInvInertia0 * Vector3::Cross( r0, cd.m_normal), r0) 
			+ Vector3::Cross( worldInvInertia1 * Vector3::Cross(r1, cd.m_normal), r1) ) );*/
		float normDiv = (invMass0 + invMass1)
			+ Vector3::Dot(cd.m_normal,
			Vector3::Cross(worldInvInertia0 * Vector3::Cross(r0, cd.m_normal), r0)
			+ Vector3::Cross(worldInvInertia1 * Vector3::Cross(r1, cd.m_normal), r1));

		float jn = -1 * (1+e) * Vector3::Dot(dv, cd.m_normal) / normDiv;

		//Corrections to linear velocity changes to take into account relative momentum change.
		//That is, a heavy object will affect a light object much more so than vice versa
		pn0.m_linearVelocity += cd.m_normal * (invMass0 * jn /** (2 - (invMass1 / totalInvMass))*/ );
		pn0.m_angularVelocity += worldInvInertia0 * Vector3::Cross(r0, cd.m_normal * jn);

		pn1.m_linearVelocity -= cd.m_normal * (invMass1 * jn /**(2 - (invMass0 / totalInvMass))*/ );
		pn1.m_angularVelocity -= worldInvInertia1 * Vector3::Cross(r1, cd.m_normal * jn);

		//SO MANY CONDITIONALS... But determined that dot product vector with itself is slower
		//than all these conditionals, according to a google link.
		// Vector3::Dot(pn0.m_linearVelocity, pn0.m_linearVelocity) < REST_TOLERANCE;
		/*if (((jn / timestep) + (Vector3::Dot(pn0.m_force, cd.m_normal) < REST_TOLERANCE)) &&*/
		/*if ((jn / timestep) + (Vector3::Dot(pn1.m_force, cd.m_normal) < REST_TOLERANCE) &&*/
		Vector3 impulseDirection = cd.m_normal * (jn / timestep);

		//TODO: Output angular velocities and sort out the odd spin issue
		/*std::cout << "Force:   " << impulseDirection << std::endl;
		std::cout << "Impulse: " << jn << std::endl;
		std::cout << "Force O1:" << pn0.m_force << std::endl;
		std::cout << "Force O2:" << pn1.m_force << std::endl;*/

		if (abs(impulseDirection.x + pn0.m_force.x) < FORCE_REST_TOLERANCE &&
			abs(impulseDirection.y + pn0.m_force.y) < FORCE_REST_TOLERANCE &&
			abs(impulseDirection.z + pn0.m_force.z) < FORCE_REST_TOLERANCE &&
			abs(pn0.m_linearVelocity.x) < REST_TOLERANCE &&
			abs(pn0.m_linearVelocity.y) < REST_TOLERANCE &&
			abs(pn0.m_linearVelocity.z) < REST_TOLERANCE &&
			abs(pn0.m_angularVelocity.x) < REST_TOLERANCE &&
			abs(pn0.m_angularVelocity.y) < REST_TOLERANCE &&
			abs(pn0.m_angularVelocity.z) < REST_TOLERANCE){
			pn0.m_rest = true;
		};

		if (abs(impulseDirection.x + pn1.m_force.x) < FORCE_REST_TOLERANCE &&
			abs(impulseDirection.y + pn1.m_force.y) < FORCE_REST_TOLERANCE &&
			abs(impulseDirection.z + pn1.m_force.z) < FORCE_REST_TOLERANCE &&
			abs(pn1.m_linearVelocity.x) < REST_TOLERANCE &&
			abs(pn1.m_linearVelocity.y) < REST_TOLERANCE &&
			abs(pn1.m_linearVelocity.z) < REST_TOLERANCE &&
			abs(pn1.m_angularVelocity.x) < REST_TOLERANCE &&
			abs(pn1.m_angularVelocity.y) < REST_TOLERANCE &&
			abs(pn1.m_angularVelocity.z) < REST_TOLERANCE){
			pn1.m_rest = true;
		};
	}

	//TANGENT IMPULSE
	{
		Vector3 tangent = dv - (cd.m_normal * Vector3::Dot(dv, cd.m_normal));
		tangent.Normalise();

		float tangDiv = (invMass0 + invMass1)
			+ Vector3::Dot( tangent,
			Vector3::Cross(pn0.m_invInertia * Vector3::Cross(r0, tangent) , r0)
			+ Vector3::Cross(pn1.m_invInertia * Vector3::Cross(r1, tangent) , r1) );

		float jt = -1 * Vector3::Dot(dv, tangent) / tangDiv;

		//TODO: Clamping here?

		//Apply contact impulse
		pn0.m_linearVelocity += tangent * (jt * invMass0);
		pn0.m_angularVelocity += worldInvInertia0 * Vector3::Cross(r0, tangent * jt);

		pn1.m_linearVelocity -= tangent * (jt * invMass1);
		pn1.m_angularVelocity -= worldInvInertia1 * Vector3::Cross(r1, tangent * jt);
	}

	pn0.m_angularVelocity = pn0.m_angularVelocity * DAMPING_FACTOR;
	pn1.m_angularVelocity = pn1.m_angularVelocity * DAMPING_FACTOR;
	
}

void CollisionAABB::GenerateAABB(Mesh* m){

	//Set default values based on the first vertex
	float lowest_x = m->vertices[0].x;
	float lowest_y = m->vertices[0].y;
	float lowest_z = m->vertices[0].z;

	float highest_x = lowest_x;
	float highest_y = lowest_y;
	float highest_z = lowest_z;

	//Find the lowest and highest vertices
	for (unsigned int i=1; i<m->numVertices; ++i){
		Vector3& vertex = m->vertices[i];

		if (vertex.x < lowest_x) lowest_x = vertex.x;
		else if (vertex.x > highest_x) highest_x = vertex.x;

		if (vertex.y < lowest_y) lowest_y = vertex.y;
		else if (vertex.y > highest_y) highest_y = vertex.y;

		if (vertex.z < lowest_z) lowest_z = vertex.z;
		else if (vertex.z > highest_z) highest_z = vertex.z;
	}

	//Setup our AABB from the values
	m_halfSize = Vector3( (highest_x - lowest_x) / 2,
		(highest_y - lowest_y) / 2,
		(highest_z - lowest_z) / 2);

	m_position = Vector3( lowest_x + m_halfSize.x,
		lowest_y + m_halfSize.y,
		lowest_z + m_halfSize.z);
}