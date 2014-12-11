#pragma once

#include "../nclgl/CollisionVolume.h"
#include "../nclgl/Plane.h"

//Possible collision detections:
//LINE VS LINE

//SPHERE VS SPHERE
//SPHERE VS PLANE
//SPHERE VS AABB
//SPHERE VS CONVEX -> TO IMPLEMENT

//AABB VS PLANE

//AABB VS CONVEX MESH HAS NOT BEEN IMPLEMENTED!!!

//CONVEX VS CONVEX

//TODO: Sort out collision data obtaining from convex vs shape collision...
//GJK SPHERE COLLISION DATA NEEDS TO ORDERED CORRECTLY WITH RESPECT TO THE CORRECT OBJECT!

class Collision {

public:

	//LINE VS LINE
	static bool LineLineIntersect(const Line& l1, const Line& l2, float* t1 = NULL, float* t2 = NULL){
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
	};

	//SPHERE VS SPHERE
	static bool SphereSphereCollision(const CollisionSphere &s0, const CollisionSphere &s1, CollisionData *collisionData = NULL) {
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

	//AABB VS AABB
	static bool AABBCollision(const CollisionAABB &cube0, const CollisionAABB &cube1) {

		//Check X Axis
		float dist = abs(cube0.m_position.x - cube1.m_position.x);
		float sum = (cube0.m_halfSize.x + cube1.m_halfSize.x);

		if (dist < sum){

			//Check Y Axis
			dist = abs(cube0.m_position.y - cube1.m_position.y);
			sum = (cube0.m_halfSize.y + cube1.m_halfSize.y);

			if (dist < sum){

				//Check Z Axis
				dist = abs(cube0.m_position.z - cube1.m_position.z);
				sum = (cube0.m_halfSize.z + cube1.m_halfSize.z);

				//Overlapped shapes
				if (dist < sum) 
					return true;
			}

		}

		return false;
	}

	//SPHERE VS PLANE
	static bool SphereInColPlane(const Plane& p, const Vector3& pos, float rad, CollisionData* cd = NULL){
		//TODO: Added abs
		float seperation = Vector3::Dot(pos, p.m_normal) - p.distance;

		//if (Vector3::Dot(pos, p.m_normal) < 0) return false;

		//TODO: Remember, this function will only return true if the sphere and plane intersect
		//Remove abs if you want anything on the wrong side of the plane to instantly be found to
		//be colliding
		if (!(abs(seperation) < rad)) return false;

		if (cd){
			cd->m_penetration = rad - seperation;
			cd->m_normal = p.m_normal; //TODO: INVERSE?
			cd->m_point = pos - (p.m_normal * seperation);
		}

		return true;
	};

	//AABB VS PLANE
	static bool AABBInColPlane(const Plane& p, const Vector3& pos, const Vector3& halfSize, CollisionData* cd = NULL){
		//Get the distance from the edge of the box to the center
		Vector3 extense = halfSize;

		float fRadius = abs(p.m_normal.x * extense.x) +
			abs(p.m_normal.y * extense.y) +
			abs(p.m_normal.z * extense.z);

		float dot = Vector3::Dot(p.m_normal, pos) - p.distance;

		if (dot > fRadius)
			return false;

		return true;

	}

	//SPHERE VS AABB
	static bool SphereAABBCollision(const CollisionSphere& sphere, const CollisionAABB& cube, CollisionData* collisionData = NULL){
		//See if highest point of sphere is below the lowest point of the node's size
		if ( (sphere.m_pos.x + sphere.m_radius) < (cube.m_position.x - cube.m_halfSize.x) ){
			return false;
		}

		//See if the lowest point of the sphere is above the highest point of the nodes size
		if ( (sphere.m_pos.x - sphere.m_radius) > (cube.m_position.x + cube.m_halfSize.x) ){
			return false;
		}

		//Check Y Axis
		if ( (sphere.m_pos.y + sphere.m_radius) < (cube.m_position.y - cube.m_halfSize.y) ){
			return false;
		}

		if ( (sphere.m_pos.y - sphere.m_radius) > (cube.m_position.y + cube.m_halfSize.y) ){
			return false;
		}

		//Check Z Axis
		if ( (sphere.m_pos.z + sphere.m_radius) < (cube.m_position.z - cube.m_halfSize.z) ){
			return false;
		}

		if ( (sphere.m_pos.z - sphere.m_radius) > (cube.m_position.z + cube.m_halfSize.z) ){
			return false;
		}

		//TODO: Spherical check is expensive... I could just omit this? Would mean it possible
		//for spheres to be within the bounds even though they arent (in rare cases...)
		//Is that a big deal for a broad phase cull though?

		//Passed "bounding box". Do a spherical check:
		//Distance between the center of the node and the centre of the colSphere
		//const float distSq = (colSphere.m_pos - into.pos).LengthSq();
		//	const float distSq = (into.pos - colSphere.m_pos).LengthSq();
		//	/*const float distSq = ((colSphere.m_pos.x - into.pos.x) * (colSphere.m_pos.x - into.pos.x)) +
		//		((colSphere.m_pos.y - into.pos.y) * (colSphere.m_pos.y - into.pos.y)) +
		//		((colSphere.m_pos.z - into.pos.z) * (colSphere.m_pos.z - into.pos.z));
		//*/
		//	const float cubeDiagonalSq = Vector3(into.halfSize, into.halfSize, into.halfSize).LengthSq();
		//	//const float cubeDiagonal = sqrtf((into.halfSize * into.halfSize) + (into.halfSize * into.halfSize) + (into.halfSize * into.halfSize));
		//
		//	//const float sumRadius = (colSphere.m_radius + (into.pos + into.halfSize).Length());
		//	//const float sumRadius = (colSphere.m_radius + (into.pos + cubeDiagonal).Length());
		//	const float sumRadiusSq = (colSphere.m_radius * colSphere.m_radius) + cubeDiagonalSq;
		//
		//	//Check that the colSphere is within range.
		//	/*if ( distSq > sumRadius * sumRadius)
		//		return false;*/
		//
		//	if ( distSq > sumRadiusSq)
		//		return false;

		return true;
	}

	//SPHERE VS CONVEX
	/*static bool SphereInConvexPolygon(const CollisionSphere& sphere, const Vector3& convexShapeCenter, const Vector3& convexShapePoints, const int numPoints){ };*/

	//POINT (PER CONVEX) VS CONVEX
	static bool PointInConvexPolygon(const Vector3& testPosition, Vector3* convexShapePoints, int numPoints) {

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

	//POINT VS PLANE
	static bool PointInPlane(const Plane& p, const Vector3& point){

		//TODO: Go through this and determine if its actually correct...
		/*const float d = Vector3::Dot(p.m_normal, point);*/

		/*const float s = d - vector3::dot( p.m_normal, point);*/
		const float s = Vector3::Dot(p.m_normal, point) - p.distance;

		if (s > 0.0f) return false;
		
		return true;
	}

	//TODO: FUCK THIS SHIT UP
	static bool PointInConcavePolygon( const Vector3* shapePoints, const int numPoints, const Vector3& testPoint) {

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

	//PLANE VS CONVEX

	//TODO: Determine if this is correct!!!
	static bool ConvexInColPlane(const Plane& p, const Vector3* shapePoints, const int numPoints, CollisionData* cd = NULL){

		//If any of the points are below the plane, 
		//then the convex shape must be contacting with the plane
		for (int i=0; i<numPoints; ++i){
			if (PointInPlane(p, shapePoints[i])){
				return true;
			}
		}
		return false;
	}

	//CONVEX VS CONVEX MESH/MESH
	static bool GJK( const Vector3* shape1Points, const int numPoints1, const Vector3& shape1Center, const Vector3& shape1size,
		const Vector3* shape2Points, const int numPoints2, const Vector3& shape2Center, Vector3& shape2Size, CollisionData* cd = NULL){

		/*std::cout << "Shape 1 points: " << std::endl;
		for (unsigned int i=0; i<numPoints1; ++i){
			std::cout << "V" << i << ": " << shape1Points[i] << std::endl;
		}

		std::cout << std::endl;

		std::cout << "Shape 2 points: " << std::endl;
		for (unsigned int i=0; i<numPoints2; ++i){
			std::cout << "V" << i << ": " << shape2Points[i] << std::endl;
		}*/

		/*std::cout << std::endl;*/

		vector<Vector3> simplex;
		simplex.reserve(4); //It will have at most 4 sides! TODO: Set max size if that is actually true

		//Get a random point from the convex hull of the minkowski difference
		Vector3 searchDirection = Support(shape1Points, numPoints1, shape2Points, numPoints2, Vector3(0,1,0));

		simplex.push_back(searchDirection);

		/*std::cout << "INITIAL SIMPLEX: " << std::endl;

		for (unsigned int i=0; i<simplex.size(); ++i)
			std::cout << "V" << i << ": " << simplex[i] << std::endl;

		std::cout << std::endl;*/
		//Search in the opposite direction to the random point we found
		searchDirection.Invert();

		int maxIterations = 50;
		int noIterations = 0;

		while (noIterations < maxIterations){

			//Find the furthest point in our search direction
			Vector3 newPoint = Support(shape1Points, numPoints1, shape2Points, numPoints2, searchDirection);

			//The furthest point in the opposite direction isnt as far as the origin,
			//therefore the minkowski sum does not encompass the origin, and therefore
			//the two shapes we are checking are not intersecting!
			if (Vector3::Dot(newPoint, searchDirection) < 0)
				return false; 

			/*std::cout << "NEW POINT: " << newPoint << std::endl;
			std::cout << "Past the origin from the point: " << searchDirection << std::endl;*/

			//Else push this new point into the simplex shape
			simplex.push_back(newPoint);

			/*std::cout << "ITERATION SIMPLEX: " << std::endl;

			for (unsigned int i=0; i<simplex.size(); ++i)
				std::cout << "V" << i << ": " << simplex[i] << std::endl;

			std::cout << std::endl;*/

			//Begin looking for the origin based on our current simplex shape
			if (DoSimplex(simplex, searchDirection)){
				if (cd){
					const float sumRadius = (shape1size * cd->m_normal).Length() + (shape2Size * cd->m_normal).Length();

					//Spherical Collision response...
					/*cd->m_normal = (shape1Center - shape2Center).Normalise();*/
					cd->m_normal = (shape2Center - shape1Center).Normalise();
					cd->m_penetration = sumRadius  - (shape1Center - shape2Center).Length();
					/*cd->m_penetration = sumRadius - sqrtf( distance between them squared );*/
					cd->m_point = (shape1Center - cd->m_normal
						* (shape1size * cd->m_normal) - cd->m_penetration * 0.5f);
						// * shape 1 radius - collision data penetration * 0.5f);
				}
				return true;
			}

			noIterations++;
			/*std::cout << "AFTER DOSIMPLEX: " << std::endl;

			for (unsigned int i=0; i<simplex.size(); ++i)
				std::cout << "V" << i << ": " << simplex[i] << std::endl;

			std::cout << std::endl;*/
		}
		return false;
	}

	//GJK/SPHERE
	static bool GJKSphere(const Vector3* shape1Points, const int numPoints1, const Vector3& spherePos, const float radius, CollisionData* cd = NULL){
		/*std::cout << std::endl;*/

		vector<Vector3> simplex;
		simplex.reserve(4); //It will have at most 4 sides! TODO: Set max size if that is actually true

		//Get a random point from the convex hull of the minkowski difference
		Vector3 searchDirection = Support(shape1Points, numPoints1, spherePos, radius, Vector3(0,1,0));

		simplex.push_back(searchDirection);

		/*std::cout << "INITIAL SIMPLEX: " << std::endl;

		for (unsigned int i=0; i<simplex.size(); ++i)
			std::cout << "V" << i << ": " << simplex[i] << std::endl;

		std::cout << std::endl;*/
		//Search in the opposite direction to the random point we found
		searchDirection.Invert();

		int maxIterations = 50;
		int noIterations = 0;

		while (noIterations < maxIterations){

			//Find the furthest point in our search direction
			Vector3 newPoint = Support(shape1Points, numPoints1, spherePos, radius, searchDirection);

			//The furthest point in the opposite direction isnt as far as the origin,
			//therefore the minkowski sum does not encompass the origin, and therefore
			//the two shapes we are checking are not intersecting!
			if (Vector3::Dot(newPoint, searchDirection) < 0)
				return false; 

			/*std::cout << "NEW POINT: " << newPoint << std::endl;
			std::cout << "Past the origin from the point: " << searchDirection << std::endl;*/

			//Else push this new point into the simplex shape
			simplex.push_back(newPoint);

			/*std::cout << "ITERATION SIMPLEX: " << std::endl;

			for (unsigned int i=0; i<simplex.size(); ++i)
				std::cout << "V" << i << ": " << simplex[i] << std::endl;

			std::cout << std::endl;*/

			//Begin looking for the origin based on our current simplex shape
			if (DoSimplex(simplex, searchDirection))
				return true;

			noIterations++;
		}
		//We couldnt determine intersection after a maximum number of iterations,
		//so we can't say two shapes are intersecting.
		return false;
	}

protected:

	//Given the points that make up the two shapes, this method computes the furthest
	//point along a supplied direction vector of the minkowski sum of the two shapes.
	static Vector3 Support(const Vector3* shape1Points, const int numPoints1, const Vector3* shape2Points, const int numPoints2, const Vector3& d){
		Vector3 oppDir = d.Inverse();

		float maxDot1 = Vector3::Dot(shape1Points[0], d);
		float maxDot2 = Vector3::Dot(shape2Points[0], oppDir);

		Vector3 maxInDir = shape1Points[0];
		Vector3 maxInOppDir = shape2Points[0];

		//Loop 1, find the vector3 that has the highest dot product with the direction
		//from shape 1.
		for (int i=1; i<numPoints1; ++i){
			float newMax = Vector3::Dot(shape1Points[i], d);

			if (maxDot1 < newMax){
				maxDot1 = newMax;
				maxInDir = shape1Points[i]; //TODO: Could just store the integer location
			}
		}

		//Loop 2, find the vector3 that has the highest dot product with the inverse direction
		//from shape 2.

		for (int i=1; i<numPoints2; ++i){
			float newMax = Vector3::Dot(shape2Points[i], oppDir);

			//total or just the highest value?
			if (maxDot2 < newMax){
				maxDot2 = newMax;
				maxInOppDir = shape2Points[i];
			}
		}

		//Return the minkowski difference between the two. (The point on the
		//minkowski sum that is the furthest in a given direction)

		/*std::cout << "DIRECTION: " << d << std::endl;
		std::cout << "MAXINDIR: " << maxInDir << std::endl;

		std::cout << "OPPDIR: " << oppDir << std::endl;
		std::cout << "MAXINOPP: " << maxInOppDir << std::endl;
		std::cout << std::endl;*/

		return maxInDir - maxInOppDir;
	}

	//Support function for CONVEX VS SPHERE
	static Vector3 Support(const Vector3* shape1Points, const int numPoints1, const Vector3& position, const float radius, const Vector3& d){

		float maxDot1 = Vector3::Dot(shape1Points[0], d);

		Vector3 maxInDir = shape1Points[0];

		//Loop 1, find the vector3 that has the highest dot product with the direction
		//from shape 1.
		for (int i=1; i<numPoints1; ++i){
			float newMax = Vector3::Dot(shape1Points[i], d);

			if (maxDot1 < newMax){
				maxDot1 = newMax;
				maxInDir = shape1Points[i]; //TODO: Could just store the integer location
			}
		}

		Vector3 oppDir = d.Inverse();
		oppDir.Normalise();
		Vector3 maxInOppDir =  position + (oppDir * radius);

		return maxInDir - maxInOppDir;
	}

	//Support function for CONVEX VS AABB
	static Vector3 Support(const Vector3* shape1Points, const int numPoints1, const Vector3& m_position, const Vector3& halfSize){

	}

	//Attempts to encompass the origin in a tetrahedron, or modifies the simplex
	//to make a better guess.
	static bool DoSimplex(std::vector<Vector3>& simplex, Vector3& d){

		//LINE SIMPLEX
		if (simplex.size() < 3){
			DoSimplexLine(simplex, d);
			return false;
		}
		//TRIANGLE SIMPLEX
		else if (simplex.size() == 3){
			DoSimplexTriangle(simplex, d);
			return false;	
		}
		//TETRAHEDRAL SIMPLEX
		else {
			//Only this function can ever calculate if the simplex holds the origin,
			//and therefore directly return it.
			return DoSimplexTetra(simplex, d); 
		}
	}

	static bool DoSimplexLine(std::vector<Vector3>& simplex, Vector3& d){

		Vector3& A = simplex[1]; //The most recently added point
		Vector3& B = simplex[0];

		Vector3 AB = B - A;	//The vector from A to B
		Vector3 AO = -A;	//The vector from A to the origin

		//The origin is on the AB side of A
		if (Vector3::Dot(AB, AO) > 0){

			//Our new simplex to check against will be the line formed from
			//A -> B. Our new direction vector to search in is the Vector 
			//perpendicular to AB in the direction of the origin.
			std::swap(A, B);
			d = Vector3::Cross( Vector3::Cross(AB, AO), AB);

		}
		//The origin is on the BA side of A
		else {

			//Our new simplex to check against will be the line formed from A
			// and AO (The direction of the origin)
			std::swap(A, B);
			simplex.erase(simplex.begin()+1);
			d = AO;
		}

		return false;
	}

	static bool DoSimplexTriangle(std::vector<Vector3>& simplex, Vector3& d){

		Vector3& A = simplex[2];	//Most recently added point
		Vector3& B = simplex[1];	//2nd Most recently...
		Vector3& C = simplex[0];	

		Vector3 AO = -A; //The vector from A to the origin

		//The vector from A to B
		Vector3 AB = B - A;

		//The vector from A to C
		Vector3 AC = C - A;

		//The vector perpendicular to the triangle (straight up)
		Vector3 ABC = Vector3::Cross(AB, AC);

		//The origin is on the outside AC side of the triangle
		if ( Vector3::Dot( Vector3::Cross(ABC, AC), AO) > 0 ){

			//The origin is on the AC side of A							
			if ( Vector3::Dot(AC, AO) > 0) {

				//Our simplex goes from C->B->A to: 
				//Our resulting simplex is A->C
				std::swap(C, A);
				std::swap(C, B);
				simplex.erase(simplex.begin()+2); //OUTPUT A LINE

				//The next vector needs to be checked against is the 
				d = Vector3::Cross( Vector3::Cross( AC, AO), AC );
			}

			//SPECIAL CASE (C&P)
			else {

				//The origin is on the AB side of the triangle
				if (Vector3::Dot(AB, AO) > 0){

					//Our simplex goes from C->B->A to
					//Our resulting simplex should be A->B
					std::swap(A, C);
					simplex.erase(simplex.begin()+2); //OUTPUT A LINE
					d = Vector3::Cross( Vector3::Cross( AB, AO), AB );

				} 
				//The origin is the other side of A with respect to the triangle
				else {

					//Our resulting simplex will be A We need to remove our old
					//value for A after we move it, so that the algorithm doesnt call
					//the wrong method the next time around!

					//Our simplex goes from C->B->A to
					// A
					std::swap(C, A);
					simplex.erase(simplex.begin()+2); //OUTPUT A POINT!
					simplex.erase(simplex.begin()+1); //OUTPUT A POINT!

					d = AO; 
				}

			}

			//The origin is on the inside AC side of the triangle
		} else if ( Vector3::Dot( Vector3::Cross( AB, ABC ), AO) > 0){

			//SPECIAL CASE (C&P)
			//The origin is on the AB side of the triangle
			if (Vector3::Dot(AB, AO) > 0){

				//Our simplex goes from C->B->A to
				//Our resulting simplex should be A->B
				//simplex[0] = simplex[2]; //Set A, B is already in place
				std::swap(A, C);
				simplex.erase(simplex.begin()+2); //OUTPUT A LINE

				d = Vector3::Cross( Vector3::Cross( AB, AO), AB );

			} 
			//The origin is the other side of A with respect to the triangle
			else {

				//Our resulting simplex will be A. We need to remove our old
				//value for A after we move it, so that the algorithm doesnt call
				//the wrong method the next time around!

				//Our simplex goes from C->B->A to
				//Resulting simplex should be A
				std::swap(C, A);
				simplex.erase(simplex.begin()+2); //OUTPUT A POINT
				simplex.erase(simplex.begin()+1); //OUTPUT A POINT

				d = AO;
			}

			//The origin is somewhere above the triangle
		} else if ( Vector3::Dot( ABC, AO ) > 0 ){

			//From C->B->A
			//Our resulting simplex will be A->B->C
			 //WE ARE OUTPUTTING A TRIANGLE!
			std::swap(A, C);
			d = ABC;

		}

		//The origin is somewhere below the triangle
		else {

			//Our simplex goes from C->B->A to
			//Our resulting simplex will be A->C->B
			std::swap(C, B);
			std::swap(A, B);

			d = ABC.Inverse(); //WE ARE OUTPUTTING A TRIANGLE
		}

		return false;
	}

	//TODO: COMPLETE THIS FUNCTION, ALL THE WORKING HAS BEEN DONE... JUST TRUST THAT IT WORKS, WRITE THE CODE
	// AND TEST IT IN A CONTROLLED ENVIRONMENT! IF IT WORKS THEN HAPPY FREAKING DAYS, ELSE GET DEBUGGING, IT SHOULDNT
	// TAKE TOO LONG! :)
	static bool DoSimplexTetra(std::vector<Vector3>& simplex, Vector3& d){

		//3 ifs to determine which face you're closest to, then just do the triangle case for each one?
		Vector3& A = simplex[3]; //The most recent point!
		Vector3& B = simplex[2];
		Vector3& C = simplex[1];
		Vector3& D = simplex[0];

		//It might be easier just to write the simple triangle checks!!!!!!
		Vector3 AO = -A;

		//Adapted from code found at: http://vec3.ca/gjk/implementation/

		Vector3 AB = B - A;
		Vector3 AC = C - A;

		Vector3 ABC = Vector3::Cross(AB, AC);

		Vector3 AD, ACD, ADB;

		//CASE 1:
		if ( Vector3::Dot( ABC, AO) > 0 ){
			//In front of triangle ABC
			goto check_face;
		}

		//CASE 2:
		AD = D - A;
		ACD = Vector3::Cross( AC, AD );

		if ( Vector3::Dot( ACD, AO) > 0 ){

			//In front of triangle ACD
			B = C;
			C = D;

			AB = AC;
			AC = AD;

			ABC = ACD;

			goto check_face;
		}

		//CASE 3:
		ADB = Vector3::Cross( AD, AB );

		if ( Vector3::Dot( ADB, AO ) > 0 ){

			//In front of the triangle ADB
			C = B;
			B = D;

			AC = AB;
			AB = AD;

			ABC = ADB;

			goto check_face;
		}

		//Behind all three faces, the origin is in the tetrahedron, we're done
		return true;


check_face:

		//We have a CCW wound triangle ABC
		//the point is in front of this triangle
		//it is NOT "below" edge BC
		//It is NOT "above" the plane through A thats parallel to BC
		Vector3 ABP = Vector3::Cross( AB, ABC );

		if ( Vector3::Dot( ABP, AO ) > 0){

			C = B;
			B = A;

			d = Vector3::Cross( Vector3::Cross(AB, AO), AB );

			//n = 2 !?
			simplex.erase(simplex.begin()+3);
			simplex.erase(simplex.begin()+2);

			return false;
		}

		Vector3 ACP = Vector3::Cross( ABC, AC );

		if (Vector3::Dot(ACP, AO) > 0){

			B = A;

			d = Vector3::Cross( Vector3::Cross( AC, AO), AC );

			//n = 2!?
			simplex.erase(simplex.begin()+3);
			simplex.erase(simplex.begin()+2);

			return false;
		}

		D = C;
		C = B;
		B = A;

		d = ABC;

		//n = 3?
		simplex.erase(simplex.begin()+3);

		return false;

		/*THEY SHOULD START WITH A
		They should be ADC, ACB, and ABD*/

		//ADC
		//vector<Vector3> newSimplex;
		//newSimplex[0] = A;
		//newSimplex[1] = D;
		//newSimplex[2] = C;

		//Vector3 newDistance = d;

		////The point is not below the triangle ADC
		//if (!DoSimplexTriangle(newSimplex, newDistance)){

		//	//Then the triangle we are interested in is ADC, with the
		//	//next direction vector being the vector between the newest point
		//	//and the origin?
		//	d = newDistance; //Is this right

		//	//DCBA -> CDBA -> BDCA -> ADCB -> ADC
		//	std::swap(simplex[0], simplex[1]);
		//	std::swap(simplex[0], simplex[2]);
		//	std::swap(simplex[0], simplex[3]);

		//	simplex.erase(simplex.begin()+3);
		//	
		//}

		////ACB
		//newSimplex[0] = A;
		//newSimplex[1] = C;
		//newSimplex[2] = B;

		//newDistance = d;

		////The point is not below the triangle ACB
		//if (DoSimplexTriangle(newSimplex, newDistance)){

		//	d = newDistance;
		//	//DCBA ->  -> ACB

		//	//Then the triangle we are interested in is CBA, with the
		//	//next direction vector being ACB
		//}

		////ABD
		//newSimplex[0] = A;
		//newSimplex[1] = B;
		//newSimplex[2] = D;

		//newDistance = d;

		////The point is not below the triangle ABD
		//if (DoSimplexTriangle(newSimplex, newDistance)){
		//	d = newDistance;

		//	//DCBA -> -> ABD

		//	//The the triangle we are interested in is ABD, with the
		//	//next direction vector being ABD
		//}

		//THE POINT IS BELOW ALL TRIANGLES, REJOICE, FOR THOU HAST FOUND
		//THE COLLISION!
		return true;


	}
};