The features in the simulation are as follows:

Collision detection and response:
	SPHERE V PLANE
	SPHERE V SPHERE
	
	
Collision detection without response:
		CONVEX VS CONVEX
		CONVEX VS SPHERE
		CONVEX VS PLANE

Unfortunately due to the time constraints, I was unable to commit the time to doing EPA for obtaining collision resolution information from the termination simplex of GJK. I added a spherical estimation of the collision response data, but it doesnt look amazing... So to demonstrate my GJK implementation better I have set things fixed upon convex collision. (Piling cubes can be quite fun!)
		
AABB's have also been done as a form of collision detection, but is intended only for use as a broadphase collision volume.

The simulation also includes a recursive octree which resorts only awake nodes every frame, rather than all nodes, collapsing the tree once every 300 frames, (determined by a #define value).

The simulation also includes a bounding volume check between shapes for collision before performing a narrow phase collision detection and resolution on the shapes, unless the narrowphase and broadphase volumes for a shape are the same.

Cloth physics has also been implemented on the CPU, and is in a well balanced state.

KEY COMMANDS (PHYSICS RELATED ONLY): (which means if you mash your face you will probably find some other functionality too!)

Left Click	Fires a Cube (GJK so if it sticks don't be alarmed!)
Right Click	Fires a sphere
Middle Click	Fires a cylinder (GJK again, for demonstration purposes)

C.				Slow plant growth
V. 			Speed up plant growth

M.				Increase projectile speed
N.				Decrease projectile speed

K. 			Increase projectile size
J.				Decrease projectile size

G				Wake up all objects

, 				Line polygon mode toggle

NUMPAD1 	Toggle Octree Drawing
NUMPAD2	Toggle BroadPhase volume draw
NUMPAD3	Toggle Narrowphase volume draw
NUMPAD7	Toggle world drawing
NUMPAD9	Hide Debug interface

3				Day/Night cycle speed increase

