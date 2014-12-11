#pragma once

#include "..\nclgl\mesh.h"

class GridMesh : public Mesh
{
public:

	friend class Cloth;

	GridMesh(void);
	GridMesh(int width, int height);
	~GridMesh(void);

	void GenerateGrid(int width, int height);
	/*void InitVBO(GLuint buffer, float* data, int comps, int num, GLuint mode);
	void InitIBO(unsigned int* data, int num, unsigned int mode);*/
	virtual void Draw();

protected:
	int restart_index;
};

