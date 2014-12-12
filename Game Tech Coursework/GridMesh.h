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
	virtual void Draw();

protected:
	int restart_index;
};

