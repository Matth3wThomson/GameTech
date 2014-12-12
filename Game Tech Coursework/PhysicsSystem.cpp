#include "PhysicsSystem.h"

PhysicsSystem* PhysicsSystem::instance = 0;
float PhysicsSystem::timestep = 1.0f/120.0f;

PhysicsSystem::PhysicsSystem(void) : octTree(OctTree(4000, 5, 3, Vector3(0, 4000, 0))){
	collisionCount = 0;
	physTimer = GameTimer();
	timePassed = 0;
	updateRate = 0;
	physFrames = 0;
	timePassed = 0.0f;
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

	std::lock_guard<std::mutex> lock(nodesMutex);
	for (vector<Constraint*>::iterator i = allConstraints.begin(); i != allConstraints.end(); ++i){
		(*i)->Update(msec);
	}

	for(vector<PhysicsNode*>::iterator i = allNodes.begin(); i != allNodes.end(); ++i) {
		(*i)->Update(msec);
	}
}

void	PhysicsSystem::BroadPhase() {
	//Sort all objects into octree based on some sort of collision properties?
	std::lock_guard<std::mutex> lock(nodesMutex);
	UpdateBroadPhaseCollisionVolumes();
	octTree.Update();
}

void PhysicsSystem::UpdateBroadPhaseCollisionVolumes(){
	for (auto itr = allNodes.begin(); itr != allNodes.end(); ++itr){

		CollisionVolume* colVol = (*itr)->GetBroadPhaseVolume();

		if (!colVol)
			continue;

		switch (colVol->GetType()){
		case COLLISION_PLANE:
			{
				Plane* p = (Plane*) colVol;
				(*itr)->UpdateCollisionPlane(*p);
			}
			break;
		case COLLISION_SPHERE:
			{
				CollisionSphere* cs = (CollisionSphere*) colVol;
				(*itr)->UpdateCollisionSphere(*cs);
			}
			break;
		case COLLISION_AABB:
			{
				CollisionAABB* aabb = (CollisionAABB*) colVol;
				(*itr)->UpdateCollisionAABB(*aabb);
			}
			break;
		}

	}
}

void	PhysicsSystem::NarrowPhase(){

	//Then perform collision detection on each section of the octree recursively
	std::lock_guard<mutex> lock(nodesMutex);

	//NarrowPhaseTree(octTree.root);
	NextPhaseTree(octTree.root, true); //Perform a bounding box form of collisions on the tree.
	NarrowPhasePairs();		//Then detect narrow phase and resolve.
	//NarrowPhaseVector(allNodes);
}

void PhysicsSystem::NextPhaseTree(OctNode& on, bool boundingBox){

	if (on.octNodes.size() != 0){
		for (auto itr = on.octNodes.begin(); itr != on.octNodes.end(); itr++){
			NextPhaseTree(**itr, boundingBox);
		}
	} else {
		if (boundingBox) BroadPhaseVector(on.physicsNodes);
		else NarrowPhaseVector(on.physicsNodes);
	}
}

void PhysicsSystem::NarrowPhaseVector(std::vector<PhysicsNode*>& np){

	for (vector<PhysicsNode*>::iterator i = np.begin(); i != np.end(); ++i){
		for (auto j = i+1; j != np.end(); ++j){

			//Both objects are resting or fixed in place... WHY GO FURTHER!?
			if ( ((*i)->AtRest() && ((*j)->AtRest())) || ((*i)->GetFixed() && (*j)->GetFixed()) )
				continue;

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

						if (Collision::SphereInColPlane(*p, cs->m_pos, cs->m_radius, &cd)){
							//Wrong side of the plane... but what is the limit to the plane?
							//std::cout << "SP COLLISION\n";
							AddCollisionImpulse(*(*j), *(*i), cd);
							++collisionCount;

						}
					};

					if (cvt2 == COLLISION_AABB){
						break;
					};

					if (cvt2 == COLLISION_CONVEX){
						Plane* p = (Plane*) cv1;
						CollisionConvex* ccv2 = (CollisionConvex*) cv2;

						(*i)->UpdateCollisionPlane(*p);
						(*j)->UpdateCollisionConvex(*ccv2);

						if (Collision::ConvexInColPlane(*p, ccv2->m_collisionMesh, ccv2->m_numVertices)){
							(*j)->SetFixed(true);
						}
					}

					break;
				case COLLISION_SPHERE:
					if (cvt2 == COLLISION_PLANE) break;
					if (cvt2 == COLLISION_SPHERE){
						CollisionSphere* cs1 = dynamic_cast<CollisionSphere*>(cv1);
						CollisionSphere* cs2 = dynamic_cast<CollisionSphere*>(cv2);

						(*i)->UpdateCollisionSphere(*cs1);
						(*j)->UpdateCollisionSphere(*cs2);

						if (Collision::SphereSphereCollision(*cs1, *cs2, &cd)){
							AddCollisionImpulse(*(*i), *(*j), cd);
							++collisionCount;
						}
					};
					if (cvt2 == COLLISION_AABB) break; 

					if (cvt2 == COLLISION_CONVEX){
						CollisionSphere* cs = (CollisionSphere*) cv1;
						CollisionConvex* ccv = (CollisionConvex*) cv2;

						(*i)->UpdateCollisionSphere(*cs);
						(*j)->UpdateCollisionConvex(*ccv);

						if (Collision::GJKSphere(ccv->m_collisionMesh, ccv->m_numVertices,
							cs->m_pos, cs->m_radius)){
								(*i)->SetFixed(true);
								(*j)->SetFixed(true);
						}
					}

					break;
				case COLLISION_AABB:
					if (cvt2 == COLLISION_PLANE) break;
					if (cvt2 == COLLISION_SPHERE) break;
					if (cvt2 == COLLISION_AABB) break; 

					break;
				case COLLISION_CONVEX:
					if (cvt2 == COLLISION_CONVEX){
						CollisionConvex* ccv1 = (CollisionConvex*) cv1;
						CollisionConvex* ccv2 = (CollisionConvex*) cv2;

						(*i)->UpdateCollisionConvex(*ccv1);
						(*j)->UpdateCollisionConvex(*ccv2);

						if (Collision::GJK(ccv1->m_collisionMesh, ccv1->m_numVertices, ccv1->m_pos, 
							(*i)->GetScale(), ccv2->m_collisionMesh, ccv2->m_numVertices, ccv2->m_pos,
							(*j)->GetScale())){
								std::cout << "COLLISION WHOOPIE! " << std::endl;
								(*i)->SetFixed(true);
								(*j)->SetFixed(true);
						}
						break;
					} else if (cvt2 == COLLISION_PLANE){
						CollisionConvex* ccv1 = (CollisionConvex*) cv1;
						Plane* p = (Plane*) cv2;

						(*i)->UpdateCollisionConvex(*ccv1);
						(*j)->UpdateCollisionPlane(*p);

						if (Collision::ConvexInColPlane(*p, ccv1->m_collisionMesh, ccv1->m_numVertices)){
							std::cout << "CONVEX VS PLANE" << std::endl;
							(*i)->SetFixed(true);
						}
						break;
					} else if (cvt2 == COLLISION_SPHERE){
						CollisionConvex* ccv1 = (CollisionConvex*) cv1;
						CollisionSphere* cs = (CollisionSphere*) cv2;

						(*i)->UpdateCollisionConvex(*ccv1);
						(*j)->UpdateCollisionSphere(*cs);

						if (Collision::GJKSphere(ccv1->m_collisionMesh, ccv1->m_numVertices, cs->m_pos, cs->m_radius)){
							std::cout << "SPHERE VS CONVEX" << std::endl;
							(*j)->SetFixed(true);
						}
						break;

					}
				}
			}
		}
	}
}

//Performs bounding box check collisions for objects, and adds pairs of objects to
//the collision pairs set if they are found to overlap
void PhysicsSystem::BroadPhaseVector(std::vector<PhysicsNode*>& np){

	for (vector<PhysicsNode*>::iterator i = np.begin(); i != np.end(); ++i){
		for (auto j = i+1; j != np.end(); ++j){

			//Both objects are resting or fixed in place... WHY GO FURTHER!?
			/*if ( ((*i)->AtRest() && ((*j)->AtRest())) || ((*i)->GetFixed() && (*j)->GetFixed()) )*/
			if ( ((*i)->AtRest() || (*i)->GetFixed()) && ((*j)->AtRest() || (*j)->GetFixed() ) )
				continue;

			CollisionVolume* cv1 = (*i)->GetBroadPhaseVolume();
			CollisionVolume* cv2 = (*j)->GetBroadPhaseVolume();

			//Only continue if they both have collision volumes
			if (cv1 && cv2){

				//If both of the objects have narrow phase volumes that are equal to their
				//broad phase volumes, then we may aswell just check them for collision at the narrow
				//phase stage!
				if ( (cv1 == (*i)->GetNarrowPhaseVolume()) && (cv2 == (*j)->GetNarrowPhaseVolume()) ){
					collisionPairs.insert(COLLISION_PAIR(*i, *j));
					continue;
				}
				CollisionVolumeType cvt1 = cv1->GetType();
				CollisionVolumeType cvt2 = cv2->GetType();

				//Find out what type our first object is, and then cast accordingly
				//and update its collision volume
				switch (cvt1){

					//PLANE COLLISIONS
				case COLLISION_PLANE:
					{

						Plane* p = dynamic_cast<Plane*>(cv1);
						(*i)->UpdateCollisionPlane(*p);

						//PLANE VS SPHERE
						if (cvt2 == COLLISION_SPHERE){
							CollisionSphere* cs = dynamic_cast<CollisionSphere*>(cv2);

							(*j)->UpdateCollisionSphere(*cs);

							//Sphere in plane is the wrong way around...
							if (Collision::SphereInColPlane(*p, cs->m_pos, cs->m_radius))
								collisionPairs.insert(COLLISION_PAIR(*i, *j));
						}

						//PLANE VS AABB
						else if (cvt2 == COLLISION_AABB){
							CollisionAABB* aabb = (CollisionAABB*) cv2;

							(*j)->UpdateCollisionAABB(*aabb);

							if (Collision::AABBInColPlane(*p, aabb->m_position, aabb->m_halfSize))
								collisionPairs.insert(COLLISION_PAIR(*i, *j));						

						};
					}
					break;

					//SPHERE COLLISION
				case COLLISION_SPHERE:
					{
						CollisionSphere* cs1 = dynamic_cast<CollisionSphere*>(cv1);
						(*i)->UpdateCollisionSphere(*cs1);

						//SPHERE VS PLANE
						if (cvt2 == COLLISION_PLANE){

							Plane* p = (Plane*) (cv2);

							(*j)->UpdateCollisionPlane(*p);

							if (Collision::SphereInColPlane(*p, cs1->m_pos, cs1->m_radius)){
								collisionPairs.insert(COLLISION_PAIR(*i, *j));
							}
						}

						//SPHERE VS SPHERE
						else if (cvt2 == COLLISION_SPHERE){

							CollisionSphere* cs2 = dynamic_cast<CollisionSphere*>(cv2);

							(*j)->UpdateCollisionSphere(*cs2);

							if (Collision::SphereSphereCollision(*cs1, *cs2)){
								collisionPairs.insert(COLLISION_PAIR(*i, *j));
							}
						}

						//SPHERE VS AABB
						else if (cvt2 == COLLISION_AABB){

							CollisionAABB* aabb = (CollisionAABB*) cv2;

							(*j)->UpdateCollisionAABB(*aabb);

							if (Collision::SphereAABBCollision(*cs1, *aabb)){
								collisionPairs.insert(COLLISION_PAIR(*i, *j));
							}
						}
					}
					break;

					//AABB COLLISION
				case COLLISION_AABB:
					{
						CollisionAABB* aabb = (CollisionAABB*) cv1;
						(*i)->UpdateCollisionAABB(*aabb);

						//AABB PLANE
						if (cvt2 == COLLISION_PLANE){

							Plane* p = (Plane*) cv2;
							(*j)->UpdateCollisionPlane(*p);

							if (Collision::AABBInColPlane(*p, aabb->m_position, aabb->m_halfSize)){
								collisionPairs.insert(COLLISION_PAIR(*i, *j));
							}

						}
						//AABB VS SPHERE
						else if (cvt2 == COLLISION_SPHERE){

							CollisionSphere* cs = (CollisionSphere*) cv2;
							(*j)->UpdateCollisionSphere(*cs);

							if (Collision::SphereAABBCollision(*cs, *aabb)){
								collisionPairs.insert(COLLISION_PAIR(*i, *j));
							}
						}
						//AABB VS AABB
						else if (cvt2 == COLLISION_AABB){

							CollisionAABB* aabb2 = (CollisionAABB*) cv2;
							(*j)->UpdateCollisionAABB(*aabb2);

							if (Collision::AABBCollision(*aabb, *aabb2)){
								collisionPairs.insert(COLLISION_PAIR(*i, *j));
							}
						}
					}
					break;

				}
			}
		}
	}
}

void PhysicsSystem::NarrowPhasePairs(){

	for (auto itr = collisionPairs.begin(); itr != collisionPairs.end(); ++itr){

		CollisionVolume* cv1 = (*itr).first->GetNarrowPhaseVolume();
		CollisionVolume* cv2 = (*itr).second->GetNarrowPhaseVolume();

		//If an object has no narrow phase volume then we cant determine a collision
		//between them
		if ( !(cv1 && cv2) ) continue; //If they dont have NP col vols then skip the pairing...

		CollisionVolumeType cvt1 = cv1->GetType();
		CollisionVolumeType cvt2 = cv2->GetType();

		CollisionData cd;

		//Currently the exact same thing is done for every collision... If this remains true
		//after having sorted all the winding issues out then could simply use a boolean?

		switch (cvt1){
			//PLANE
		case COLLISION_PLANE:
			{
				Plane* p = (Plane*) cv1;
				(*itr).first->UpdateCollisionPlane(*p);

				//VS SPHERE
				if (cvt2 == COLLISION_SPHERE){
					CollisionSphere* cs = (CollisionSphere*) cv2;
					(*itr).second->UpdateCollisionSphere(*cs);

					//Sphere plane collision data is the wrong way around...
					if (Collision::SphereInColPlane(*p, cs->m_pos, cs->m_radius, &cd)){
						AddCollisionImpulse(*(*itr).second, *(*itr).first, cd);
						++collisionCount;
						(*itr).first->noOfCollisions++;
						(*itr).second->noOfCollisions++;

					}
				}
				//VS CONVEX
				else if (cvt2 == COLLISION_CONVEX){
					CollisionConvex* ccv = (CollisionConvex*) cv2;
					(*itr).second->UpdateCollisionConvex(*ccv);

					if (Collision::ConvexInColPlane(*p, ccv->m_collisionMesh, ccv->m_numVertices, &cd)){
						(*itr).second->SetFixed(true); //TODO: Remove
						/*AddCollisionImpulse(*(*itr).first, *(*itr).second, cd);*/
						++collisionCount;
						(*itr).first->noOfCollisions++;
						(*itr).second->noOfCollisions++;
					}
				}
			}
			break;

			//SPHERE
		case COLLISION_SPHERE:
			{
				CollisionSphere* cs = (CollisionSphere*) cv1;
				(*itr).first->UpdateCollisionSphere(*cs);

				//VS PLANE
				if (cvt2 == COLLISION_PLANE){
					Plane* p = (Plane*) cv2;
					(*itr).second->UpdateCollisionPlane(*p);

					if (Collision::SphereInColPlane(*p, cs->m_pos, cs->m_radius, &cd)){
						AddCollisionImpulse(*(*itr).first, *(*itr).second, cd);
						++collisionCount;
						(*itr).first->noOfCollisions++;
						(*itr).second->noOfCollisions++;
					}
				}
				//VS SPHERE
				else if (cvt2 == COLLISION_SPHERE){
					CollisionSphere* cs2 = (CollisionSphere*) cv2;
					(*itr).second->UpdateCollisionSphere(*cs2);

					if (Collision::SphereSphereCollision(*cs, *cs2, &cd)){
						AddCollisionImpulse(*(*itr).first, *(*itr).second, cd);
						++collisionCount;
						(*itr).first->noOfCollisions++;
						(*itr).second->noOfCollisions++;
					}
				}
				//VS CONVEX
				else if (cvt2 == COLLISION_CONVEX){
					CollisionConvex* ccv = (CollisionConvex*) cv2;
					(*itr).second->UpdateCollisionConvex(*ccv);

					if (Collision::GJKSphere(ccv->m_collisionMesh, ccv->m_numVertices, cs->m_pos,
						cs->m_radius, &cd)){
							(*itr).first->SetFixed(true);	//TODO: REMOVE THIS ONCE CD IS DONE
							(*itr).second->SetFixed(true);
							//AddCollisionImpulse(*(*itr).first, *(*itr).second, cd);
							++collisionCount;
							(*itr).first->noOfCollisions++;
							(*itr).second->noOfCollisions++;
					}
				}
			}
			break;

			//CONVEX
		case COLLISION_CONVEX:
			{
				CollisionConvex* ccv1 = (CollisionConvex*) cv1;
				(*itr).first->UpdateCollisionConvex(*ccv1);

				//VS PLANE
				if (cvt2 == COLLISION_PLANE){
					Plane* p = (Plane*) cv2;
					(*itr).second->UpdateCollisionPlane(*p);

					if (Collision::ConvexInColPlane(*p, ccv1->m_collisionMesh, ccv1->m_numVertices,
						&cd)){
							(*itr).first->SetFixed(true);	//TODO: Remove this once CD is DONE
							(*itr).second->SetFixed(true);
							//AddCollisionImpulse(*(*itr).first, *(*itr).second, cd);
							++collisionCount;
							(*itr).first->noOfCollisions++;
							(*itr).second->noOfCollisions++;
					}
				}
				//VS SPHERE
				else if (cvt2 == COLLISION_SPHERE){
					CollisionSphere* cs = (CollisionSphere*) cv2;
					(*itr).second->UpdateCollisionSphere(*cs);

					if (Collision::GJKSphere(ccv1->m_collisionMesh, ccv1->m_numVertices, cs->m_pos,
						cs->m_radius, &cd)){
							(*itr).first->SetFixed(true);	//TODO: Remove this once CR is DONE
							(*itr).second->SetFixed(true);
							//AddCollisionImpulse(*(*itr).first, *(*itr).second, cd);
							++collisionCount;
							(*itr).first->noOfCollisions++;
							(*itr).second->noOfCollisions++;
					}
				}
				//VS CONVEX!
				else if (cvt2 == COLLISION_CONVEX){
					CollisionConvex* ccv2 = (CollisionConvex*) cv2;
					(*itr).second->UpdateCollisionConvex(*ccv2);

					if (Collision::GJK(ccv1->m_collisionMesh, ccv1->m_numVertices, ccv1->m_pos,
						(*itr).first->GetScale(), ccv2->m_collisionMesh, ccv2->m_numVertices, ccv2->m_pos,
						(*itr).second->GetScale(), &cd)){
							(*itr).first->SetFixed(true);	//TODO: Remove this once CR is DONE
							(*itr).second->SetFixed(true);
							/*AddCollisionImpulse(*(*itr).first, *(*itr).second, cd);*/
							++collisionCount;
							(*itr).first->noOfCollisions++;
							(*itr).second->noOfCollisions++;
					}
				}
			}
			break;
		}
	}

	collisionPairs.clear();
}

void PhysicsSystem::WakeAllNodes(){
	std::lock_guard<std::mutex> lock(nodesMutex);
	for (auto itr = allNodes.begin(); itr != allNodes.end(); ++itr){
		(*itr)->Wake();
	}
}

void	PhysicsSystem::AddNode(PhysicsNode* n){
	std::lock_guard<mutex> lock(nodesMutex);
	allNodes.push_back(n);
	octTree.AddPhysicsNode(n);
}

void	PhysicsSystem::RemoveNode(PhysicsNode* n) {
	std::lock_guard<mutex> lock(nodesMutex);
	for(vector<PhysicsNode*>::iterator i = allNodes.begin(); i != allNodes.end(); ++i) {
		if((*i) == n) {
			allNodes.erase(i);
			return;
		}
	}
}

void PhysicsSystem::AddConstraint(Constraint* c){
	std::lock_guard<mutex> lock(nodesMutex);
	allConstraints.push_back(c);
}

void PhysicsSystem::RemoveConstraint(Constraint* c){
	std::lock_guard<mutex> lock(nodesMutex);
	for (auto itr = allConstraints.begin(); itr != allConstraints.end(); ++itr){
		if ((*itr) == c){
			allConstraints.erase(itr);
			return;
		}
	}
}


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

	const Matrix3 worldInvInertia0 = pn0.m_invInertia;
	const Matrix3 worldInvInertia1 = pn1.m_invInertia;

	//Both immovable, dont continue
	if ( (invMass0+invMass1) == 0.0f )
		return;

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

	pn1.m_position -= cd.m_normal * cd.m_penetration * (invMass1 / totalInvMass);

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
		pn0.m_linearVelocity += cd.m_normal * (invMass0 * jn);
		pn0.m_angularVelocity += worldInvInertia0 * Vector3::Cross(r0, cd.m_normal * jn);

		pn1.m_linearVelocity -= cd.m_normal * (invMass1 * jn);
		pn1.m_angularVelocity -= worldInvInertia1 * Vector3::Cross(r1, cd.m_normal * jn);

		//SO MANY CONDITIONALS... But determined that dot product vector with itself is slower
		//than all these conditionals, according to a google link.
		Vector3 impulseDirection = cd.m_normal * (jn / timestep);

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

		//Apply contact impulse
		pn0.m_linearVelocity += tangent * (jt * invMass0);
		pn0.m_angularVelocity += worldInvInertia0 * Vector3::Cross(r0, tangent * jt);

		pn1.m_linearVelocity -= tangent * (jt * invMass1);
		pn1.m_angularVelocity -= worldInvInertia1 * Vector3::Cross(r1, tangent * jt);
	}

	pn0.m_angularVelocity = pn0.m_angularVelocity * DAMPING_FACTOR;
	pn1.m_angularVelocity = pn1.m_angularVelocity * DAMPING_FACTOR;

	if (pn0.lastCollided == &pn1)
		if (pn0.m_linearVelocity.LengthSq() < REST_TOLERANCE &&
			pn0.m_angularVelocity.LengthSq() < REST_TOLERANCE){
				pn0.m_rest = true;
		}

		if (pn1.lastCollided == &pn0)
			if (pn1.m_linearVelocity.LengthSq() < REST_TOLERANCE &&
				pn1.m_angularVelocity.LengthSq() < REST_TOLERANCE){
					pn1.m_rest = true;
			}

			pn0.lastCollided = &pn1;
			pn1.lastCollided = &pn0;

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