#include "GridMesh.h"


GridMesh::GridMesh(void)
{
	
}

GridMesh::GridMesh(int width, int height){
	GenerateGrid(width, height);
}

GridMesh::~GridMesh(void)
{
}

//void GridMesh::GenerateGrid(int width, int height){
//	int loop_size = 2*height + 1;
//
//	int num_verts = width*height;
//	int num_indices = (width - 1)*loop_size;
//
//	float* verts = new float[3*num_verts];
//	float* norms = new float[3*num_verts];
//	float* texcoords = new float[2*num_verts];
//	unsigned int* indices = new unsigned int[num_indices];
//
//	type = GL_TRIANGLE_STRIP;
//	for (int x = 0; x < width; x++) {
//		int loops = x*loop_size;
//		for (int y = 0; y < height; y++) {
//			int offset = y*width + x;
//
//			if (x != width - 1)
//				indices[loops + 2*y + 1] = offset;
//			if (x != 0)
//				indices[loops - loop_size + 2*y] = offset;
//
//			verts[3*offset + 0] = 2*(x*1.0f/(width-1)) - 1;
//			verts[3*offset + 1] = 0;
//			verts[3*offset + 2] = 2*(y*1.0f/(height-1)) - 1;
//
//			norms[3*offset + 0] = 0;
//			norms[3*offset + 1] = 1;
//			norms[3*offset + 2] = 0;
//
//			texcoords[2*offset + 0] = x*1.0f/(width-1);
//			texcoords[2*offset + 1] = y*1.0f/(height-1);
//		}
//		if (x != width - 1)
//			indices[loops + loop_size - 1] = width*height;
//	}
//
//	restart_index = width*height;
//
//	glBindVertexArray(arrayObject);
//	InitVBO(VERTEX_BUFFER, (float*)verts, 3, num_verts, GL_DYNAMIC_DRAW);
//	InitVBO(NORMAL_BUFFER, (float*)norms, 3, num_verts, GL_DYNAMIC_DRAW);
//	InitVBO(TEXTURE_BUFFER, (float*)texcoords, 2, num_verts, GL_DYNAMIC_DRAW);
//	InitIBO(indices, num_indices, GL_DYNAMIC_DRAW);
//	glBindVertexArray(0);
//
//	delete[] verts;
//	delete[] norms;
//	delete[] texcoords;
//	delete[] indices;
//}

void GridMesh::GenerateGrid(int width, int height){
	int loop_size = 2*height + 1;

	numVertices = width*height;
	numIndices = (width -1) * loop_size;

	vertices = new Vector3[numVertices];
	normals = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new unsigned int[numIndices];

	type = GL_TRIANGLE_STRIP;
	for (int x = 0; x < width; x++) {
		int loops = x*loop_size;
		for (int y = 0; y < height; y++) {
			int offset = y*width + x;

			if (x != width - 1)
				indices[loops + 2*y + 1] = offset;
			if (x != 0)
				indices[loops - loop_size + 2*y] = offset;

			vertices[offset] = Vector3(2*(x*1.0f/(width-1)) - 1, 0, 2*(y*1.0f/(height-1)) - 1);
			normals[offset] =Vector3(0,1,0);
			textureCoords[offset] = Vector2(x*1.0f/(width-1), y*1.0f/(height-1));
		}
		if (x != width - 1)
			indices[loops + loop_size - 1] = width*height;
	}

	restart_index = width*height;

	BufferData();
}

void GridMesh::Draw(){
	glPrimitiveRestartIndex(restart_index);
	glEnable(GL_PRIMITIVE_RESTART);

	Mesh::Draw();

	glDisable(GL_PRIMITIVE_RESTART);
}

//void GridMesh::InitVBO(GLuint buffer, float* data, int comps, int num, GLuint mode){
//	glGenBuffers(1, &bufferObject[buffer]);
//	glBindBuffer(GL_ARRAY_BUFFER, bufferObject[buffer]);
//	glBufferData(GL_ARRAY_BUFFER, num*comps*sizeof(float), (GLvoid*)data, mode);
//	glVertexAttribPointer((GLuint) buffer, comps, GL_FLOAT, GL_FALSE, 0, 0);
//	glEnableVertexAttribArray((GLuint) buffer);
//}
//
//void GridMesh::InitIBO(unsigned int* data, int num, unsigned int mode) {
//	numIndices = num;
//	glGenBuffers(1, &bufferObject[INDEX_BUFFER]);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObject[INDEX_BUFFER]);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num * sizeof(GLuint), (GLvoid*)data, mode);
//}