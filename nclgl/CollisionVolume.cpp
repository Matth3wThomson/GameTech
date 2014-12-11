#include "CollisionVolume.h"

//This isnt appropriate as from the mesh vertices we have no idea of the winding, or way in which
//those vertices are used to create the overall Geometry... Therefore convex polygons wont work correctly
//unless the mesh is perfectly made...
CollisionConvex::CollisionConvex(Mesh* m){
	type = COLLISION_CONVEX;

	if (OBJMesh* om =  dynamic_cast<OBJMesh*>(m)){

		//Then we only care about the first child of the obj mesh... sorry!
		m_numVertices = om->children[0]->numVertices;

		//Maybe I cant use memcpy?
		//memcpy( m_collisionMesh, om->children[0]->vertices, m_numVertices*sizeof(Vector3));

		m_collisionMesh = new Vector3[m_numVertices];
		for (int i=0; i<m_numVertices; ++i){
			m_collisionMesh[i] = om->children[0]->vertices[i];
		}

		//This is sketchy... If the objmesh is modified at all then this pointer will be invalidated!
		m_mesh = om->children[0];

	} else {

		//TODO: This doesnt work if the mesh itself is an OBJMesh -.-
		m_numVertices = m->numVertices;

		m_collisionMesh = new Vector3[m->numVertices];
		for (int i=0; i<m_numVertices; ++i){
			m_collisionMesh[i] = m->vertices[i];
		}

		m_mesh = m;
	}

}

//This method updates a collision convex's vertices relative to the supplied
//position, orientation and scale
void CollisionConvex::Update(const Vector3& position, const Quaternion& orientation, const Vector3& scale){
	m_pos = position;

	Matrix4 Transform = Matrix4::Translation(position) *
		orientation.ToMatrix() *
		Matrix4::Scale(scale);

	//DO we have to use matrices here?
	for (int i=0; i<m_numVertices; ++i){
		m_collisionMesh[i] = Transform * m_mesh->vertices[i];
	}
}