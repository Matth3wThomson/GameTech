The features in the simulation are as follows:

Collision detection and response:
	SPHERE V PLANE
	SPHERE V SPHERE
	
	
Collision detection without response:
		CONVEX VS CONVEX
		CONVEX VS SPHERE
		CONVEX VS PLANE
		
AABB's have also been done as a form of collision detection, but is intended only for use as a broadphase collision volume.

The simulation also includes a recursive octree which resorts only awake nodes every frame, rather than all nodes.

The simulation also includes a bounding volume check between shapes for collision before performing a narrow phase collision detection and resolution on the shapes, unless the narrowphase and broadphase volumes for a shape are
the same.
