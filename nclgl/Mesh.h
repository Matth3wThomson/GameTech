#pragma once
#include "OGLRenderer.h"

enum MeshBuffer {
	VERTEX_BUFFER, COLOUR_BUFFER, TEXTURE_BUFFER, NORMAL_BUFFER,
	TANGENT_BUFFER, INDEX_BUFFER, MAX_BUFFER
};

class CollisionAABB;

class Mesh
{
public:

	friend class CollisionAABB;
	friend class CollisionConvex;

	Mesh(void);
	virtual ~Mesh(void);

	virtual void Draw();

	static Mesh* GenerateTriangle();
	static Mesh* GenerateQuad();
	static Mesh* GenerateTransQuad();
	static Mesh* GenerateBAQuad(bool normals);
	static Mesh* GenerateCylinder(int accuracy, float topOffset = 1.0f);
	//static Mesh* GenerateCube();

	void SetTexture(GLuint tex){ texture = tex; }
	GLuint GetTexture() const { return texture; }

	void SetBumpMap(GLuint tex){ bumpTexture = tex; }
	GLuint GetBumpMap(){ return bumpTexture; }

	//Note this does not include texture memory!
	unsigned int GetGPUMemoryUsed(){ return bytes; };

protected:
	
	void GenerateNormals();

	void GenerateTangents();
	Vector3 GenerateTangent(const Vector3& a, const Vector3& b,
		const Vector3& c, const Vector2& ta,
		const Vector2& tb, const Vector2& tc);

	void BufferData();

	GLuint arrayObject;
	GLuint bufferObject[MAX_BUFFER];
	GLuint numVertices;
	GLuint type;

	GLuint texture;
	GLuint numIndices;
	GLuint bumpTexture;

	Vector3* vertices;
	Vector3* normals;
	Vector3* tangents;
	Vector4* colours;
	Vector2* textureCoords;
	unsigned int* indices;

	unsigned int bytes;
};

