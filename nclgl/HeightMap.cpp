#include "HeightMap.h"


HeightMap::HeightMap(const std::string& name)
{
	std::ifstream file(name.c_str(), ios::binary);

	if (!file) return;

	numVertices = RAW_WIDTH * RAW_HEIGHT;
	numIndices = (RAW_WIDTH-1) * (RAW_HEIGHT-1)*6;

	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	colours = new Vector4[numVertices]; //NEW
	indices = new GLuint[numIndices];

	//Load in all char data from file
	unsigned char* data = new unsigned char[numVertices];
	file.read((char*) data, numVertices*sizeof(unsigned char));
	file.close();

	//Create our vertices and texture data from file
	for (int x = 0; x < RAW_WIDTH; ++x){
		for (int z=0; z < RAW_HEIGHT; ++z){
			int offset = (x * RAW_WIDTH) + z;

			//Scaled by a pre determined factor (modifying the terrain loaded 
			//from file)
			vertices[offset] = Vector3(x * HEIGHTMAP_X - (HEIGHTMAP_X * RAW_WIDTH * 0.5f), 
				data[offset] * HEIGHTMAP_Y, z * HEIGHTMAP_Z - (HEIGHTMAP_Z * RAW_HEIGHT * 0.5f));

			/*if (vertices[offset].y < 100.0f)
				colours[offset] = Vector4(0.2f, 0.2f, 0.2f, 0.2f);
			else 
				colours[offset] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);*/

			colours[offset] = Vector4(vertices[offset].y / 200.0f,
				vertices[offset].y / 200.0f, vertices[offset].y / 200.0f,
				vertices[offset].y / 200.0f);

			textureCoords[offset] = Vector2(
				x * HEIGHTMAP_TEX_X, z * HEIGHTMAP_TEX_Z);
		}
	}

	delete data;

	numIndices = 0;

	for (int x = 0; x < RAW_WIDTH-1; ++x){
		for (int z = 0; z < RAW_HEIGHT-1; ++z){
			int a = (x * (RAW_WIDTH)) + z;
			int b = ((x+1) * RAW_WIDTH) + z;
			int c = ((x+1) * RAW_WIDTH) + (z+1);
			int d = (x * RAW_WIDTH) + (z+1);

			indices[numIndices++] = c;
			indices[numIndices++] = b;
			indices[numIndices++] = a;

			indices[numIndices++] = a;
			indices[numIndices++] = d;
			indices[numIndices++] = c;
		}
	}


	GenerateNormals(); //For lighting calculations!
	GenerateTangents(); //For bump mapping! :D
	BufferData();
}


HeightMap::~HeightMap(void)
{
}
