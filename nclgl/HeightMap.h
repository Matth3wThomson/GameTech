#pragma once

#include "..\nclgl\mesh.h"

#include <string>
#include <iostream>
#include <fstream>

#define RAW_WIDTH 257
#define RAW_HEIGHT 257

#define HEIGHTMAP_X 16.0f
#define HEIGHTMAP_Z 16.0f
#define HEIGHTMAP_Y 1.5f

#define HEIGHTMAP_TEX_X 1.0f / 16.0f
#define HEIGHTMAP_TEX_Z 1.0f / 16.0f

/*
Notes:
	-Changed constructor to maintain the origin as the center of the
	 heightmap
*/

class HeightMap :
	public Mesh
{
public:
	//TODO: Why no reference?
	HeightMap(const std::string& name);
	~HeightMap(void);
};

