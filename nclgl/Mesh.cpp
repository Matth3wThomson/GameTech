#include "Mesh.h"


Mesh::Mesh(void)
{
	for (int i=0; i<MAX_BUFFER; ++i)
		bufferObject[i] = 0;

	glGenVertexArrays(1, &arrayObject);

	indices = NULL;

	numIndices = 0;
	numVertices = 0;
	texture = 0;
	bumpTexture = 0;

	vertices = NULL;
	normals = NULL;
	tangents = NULL;
	colours = NULL;
	textureCoords = NULL;
	type = GL_TRIANGLES;

}


Mesh::~Mesh(void)
{
	glDeleteVertexArrays(1, &arrayObject);
	glDeleteBuffers(MAX_BUFFER, bufferObject);
	glDeleteTextures(1, &texture);
	glDeleteTextures(1, &bumpTexture);

	delete[] vertices;
	delete[] normals;
	delete[] tangents;
	delete[] colours;
	delete[] textureCoords;
	delete[] indices;
}

Mesh* Mesh::GenerateTriangle() {
	Mesh* m = new Mesh();
	m->numVertices = 3;

	m->vertices = new Vector3[m->numVertices];
	m->vertices[0] = Vector3(0.0f, 0.5f, 0.0f);
	m->vertices[1] = Vector3(0.5f, -0.5f, 0.0f);
	m->vertices[2] = Vector3(-0.5f, -0.5f, 0.0f);

	m->textureCoords = new Vector2[m->numVertices];
	m->textureCoords[0] = Vector2(0.5f, 0.0f);
	m->textureCoords[1] = Vector2(1.0f, 1.0f);
	m->textureCoords[2] = Vector2(0.0f, 1.0f);

	m->colours = new Vector4[m->numVertices];
	m->colours[0] = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	m->colours[1] = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
	m->colours[2] = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

	m->BufferData();
	return m;
}

Mesh* Mesh::GenerateQuad(){
	Mesh* m = new Mesh();

	m->numVertices = 4;
	m->type = GL_TRIANGLE_STRIP;

	m->vertices = new Vector3[m->numVertices];
	m->textureCoords = new Vector2[m->numVertices];
	m->colours = new Vector4[m->numVertices];
	m->normals = new Vector3[m->numVertices];
	m->tangents = new Vector3[m->numVertices];

	m->vertices[0] = Vector3(-1.0f, -1.0f, 0.0f);
	m->vertices[1] = Vector3(-1.0f, 1.0f, 0.0f);
	m->vertices[2] = Vector3(1.0f, -1.0f, 0.0f);
	m->vertices[3] = Vector3(1.0f, 1.0f, 0.0f);

	m->textureCoords[0] = Vector2(0.0f, 1.0f);
	m->textureCoords[1] = Vector2(0.0f, 0.0f);
	m->textureCoords[2] = Vector2(1.0f, 1.0f);
	m->textureCoords[3] = Vector2(1.0f, 0.0f);

	for (int i=0; i<4; ++i){
		m->colours[i] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		m->normals[i] = Vector3(0.0f, 0.0f, -1.0f);
		m->tangents[i] = Vector3(1.0f, 0.0f, 0.0f);
	}

	m->BufferData();

	return m;

}

Mesh* Mesh::GenerateTransQuad(){
	Mesh* m = new Mesh();
	m->numVertices = 4;
	m->type = GL_TRIANGLE_STRIP;

	m->vertices = new Vector3[m->numVertices];
	m->textureCoords = new Vector2[m->numVertices];
	m->colours = new Vector4[m->numVertices];

	m->vertices[0] = Vector3(-1.0f, -1.0f, 0.1f);
	m->vertices[1] = Vector3(-1.0f, 1.0f, 0.1f);
	m->vertices[2] = Vector3(1.0f, -1.0f, 0.1f);
	m->vertices[3] = Vector3(1.0f, 1.0f, 0.1f);

	m->textureCoords[0] = Vector2(0.0f, 1.0f);
	m->textureCoords[1] = Vector2(0.0f, 0.0f);
	m->textureCoords[2] = Vector2(1.0f, 1.0f);
	m->textureCoords[3] = Vector2(1.0f, 0.0f);

	for (int i=0; i<4; ++i)
		m->colours[i] = Vector4(1.0f, 1.0f, 1.0f, 0.5f);

	m->BufferData();
	return m;

}

Mesh* Mesh::GenerateBAQuad(){
	Mesh* m = new Mesh();
	m->numVertices = 4;
	m->type = GL_TRIANGLE_STRIP;

	m->vertices = new Vector3[m->numVertices];
	m->textureCoords = new Vector2[m->numVertices];
	m->colours = new Vector4[m->numVertices];

	m->vertices[0] = Vector3(-1.0f, 0.0f, 0.1f);
	m->vertices[1] = Vector3(-1.0f, 1.0f, 0.1f);
	m->vertices[2] = Vector3(1.0f, 0.0f, 0.1f);
	m->vertices[3] = Vector3(1.0f, 1.0f, 0.1f);

	m->textureCoords[0] = Vector2(0.0f, 1.0f);
	m->textureCoords[1] = Vector2(0.0f, 0.0f);
	m->textureCoords[2] = Vector2(1.0f, 1.0f);
	m->textureCoords[3] = Vector2(1.0f, 0.0f);

	for (int i=0; i<4; ++i)
		m->colours[i] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	m->BufferData();
	return m;
}

Mesh* Mesh::GenerateCylinder(int accuracy, float topOffset){

	//We must make accuracy odd!
	accuracy += 1 - accuracy%2;

	//The first circle will require
	// accuracy+1 vertices;
	// accuracy*3 indices

	//The second circle will require
	// accuracy+1 vertices
	//accuracy*3 indices

	//The strip around the outside requires
	// 6 indices per quad. accuracy * quad
	

	Mesh* m = new Mesh();

	m->numVertices = ((accuracy+1)*2) + 2;
	m->numIndices = accuracy*3*2 + accuracy * 6; //THIS NEEDS TO BE CALCULATED
	m->type = GL_TRIANGLES;
	/*m->type = GL_TRIANGLE_FAN;*/


	m->indices = new unsigned int[m->numIndices];

	m->vertices = new Vector3[m->numVertices];
	m->textureCoords = new Vector2[m->numVertices];
	m->colours = new Vector4[m->numVertices];
	m->normals = new Vector3[m->numVertices];
	m->tangents = new Vector3[m->numVertices];

	int indexCounter = 0;

	//Centre of bottom circle
	m->vertices[0] = Vector3(0,0,0);
	m->colours[0] = Vector4(1,1,1,1);
	m->textureCoords[0] = Vector2(0.5, 0.5);

	//First texture correct vertex
	m->vertices[1] = Vector3(0, 0, 1);
	m->colours[1] = Vector4(1,1,1,1);
	m->textureCoords[1] = Vector2(0,0);

	/*m->indices[indexCounter++] = 0;
	m->indices[indexCounter++] = 2;
	m->indices[indexCounter++] = 1;*/

	//Generate the bottom circle

	for (int i=0; i<accuracy-1; i++){
		m->vertices[i+2] = Vector3( (float) sin( DegToRad(((float) (i+1) / (accuracy)) * 360)), 0, (float) cos( DegToRad(((float) (i+1) / (accuracy)) * 360)));

		//m->textureCoords[i] = Vector2( sin( DegToRad(((float)i/accuracy) * 360)), cos( DegToRad(((float)i/accuracy) * 360)));
		m->textureCoords[i+2] = Vector2((float) (i+1) / (accuracy), 0);
		
		m->colours[i+2] = Vector4(1,1,1,1);
		
		//draw the shape CCW so GL can cull the face when not in view!
		m->indices[indexCounter++] = 0;
		m->indices[indexCounter++] = i+2;
		m->indices[indexCounter++] = i+1;
	}

	//Place the final texture correct vertex
	m->vertices[accuracy+1] = Vector3(0, 0, 1);
	m->colours[accuracy+1] = Vector4(1,1,1,1);
	m->textureCoords[accuracy+1] = Vector2(1,0);

	m->indices[indexCounter++] = 0;
	m->indices[indexCounter++] = accuracy+1;
	m->indices[indexCounter++] = accuracy;

	//Generate the top circle

	//Centre point
	m->vertices[accuracy+2] = Vector3(0,1,0);
	m->colours[accuracy+2] = Vector4(1,1,1,1);
	m->textureCoords[accuracy+2] = Vector2(0.5, 0.5);

	//First texture correct vertex
	m->vertices[accuracy+3] = Vector3(0, 1, 1*topOffset);
	m->colours[accuracy+3] = Vector4(1,1,1,1);
	m->textureCoords[accuracy+3] = Vector2(0,1);

	for (int i=0; i<accuracy-1; i++){
		m->vertices[i+accuracy+4] = Vector3( topOffset * (float) sin( DegToRad(((float) (i+1) / (accuracy)) * 360)), 1, topOffset* (float) cos( DegToRad(((float) (i+1) / (accuracy)) * 360)));

		//m->textureCoords[i] = Vector2( sin( DegToRad(((float)i/accuracy) * 360)), cos( DegToRad(((float)i/accuracy) * 360)));
		m->textureCoords[i+accuracy+4] = Vector2((float) (i+1) / (accuracy), 1);
		
		m->colours[i+accuracy+4] = Vector4(1,1,1,1);
		
		//draw the shape CCW so GL can cull the face when not in view!
		m->indices[indexCounter++] = accuracy+2;
		m->indices[indexCounter++] = i+accuracy+3;
		m->indices[indexCounter++] = i+accuracy+4;
	}

	//Place the final texture correct vertex
	m->vertices[accuracy+accuracy-1+4] = Vector3(0, 1, 1*topOffset);
	m->colours[accuracy+accuracy-1+4] = Vector4(1,1,1,1);
	m->textureCoords[accuracy+accuracy-1+4] = Vector2(1,1);
	
	m->indices[indexCounter++] = accuracy+2;
	m->indices[indexCounter++] = accuracy+accuracy-1+3;
	m->indices[indexCounter++] = accuracy+accuracy-1+4;

	//Generate tube around cylinder
	for (int i=1; i<accuracy+1; i++){
		m->indices[indexCounter++] = i;		//1
		m->indices[indexCounter++] = i+1;	//2
		m->indices[indexCounter++] = i+accuracy+3;	 //11
		
		m->indices[indexCounter++] = i+accuracy+3;		//11
		m->indices[indexCounter++] = i+accuracy+2;	//10
		m->indices[indexCounter++] = i;	 //1
	}

   	m->GenerateNormals();
	m->GenerateTangents();

 	m->BufferData();

	return m;

}

void Mesh::GenerateNormals(){
	if (!normals)
		normals = new Vector3[numVertices];

	for (GLuint i = 0; i<numVertices; ++i)
		normals[i] = Vector3();

	if (indices){ //Generate per-vertex normals
		for (GLuint i=0; i<numIndices; i+=3){

			unsigned int a = indices[i];
			unsigned int b = indices[i+1];
			unsigned int c = indices[i+2];

			Vector3 normal = Vector3::Cross(
				(vertices[b]-vertices[a]), (vertices[c]-vertices[a]));

			//For every face that each vertex is a part of, the normals will be accumulated
			//hence the use of +=
			normals[a] += normal;	
			normals[b] += normal;
			normals[c] += normal;
		}
	}
	else { //Just a list of triangles, so generate face normals (stored in every vertex however!)
		for (GLuint i = 0; i < numVertices; i+=3){

			Vector3& a = vertices[i];
			Vector3& b = vertices[i+1];
			Vector3& c = vertices[i+2];

			Vector3 normal = Vector3::Cross(b-a,c-a);

			normals[i] = normal;
			normals[i+1] = normal;
			normals[i+2] = normal;
		}
	}

	//Normalise all the normals once they have been computed
	for (GLuint i=0; i<numVertices; ++i)
		normals[i].Normalise();
}

void Mesh::GenerateTangents(){
	if (!tangents)
		tangents = new Vector3[numVertices];

	for (GLuint i=0; i<numVertices; ++i)
		tangents[i] = Vector3();

	if (indices){
		for (GLuint i=0; i<numIndices; i+=3){

			//TODO: unsigned?
			int a = indices[i];
			int b = indices[i+1];
			int c = indices[i+2];

			Vector3 tangent = GenerateTangent(vertices[a], vertices[b],
				vertices[c], textureCoords[a],
				textureCoords[b], textureCoords[c]);

			tangents[a] += tangent;
			tangents[b] += tangent;
			tangents[c] += tangent;
		}
	} else {
		for (GLuint i=0; i<numVertices; i+=3){
			Vector3 tangent = GenerateTangent(vertices[i], vertices[i+1],
				vertices[i+2], textureCoords[i],
				textureCoords[i+1], textureCoords[i+2]);


			//This shouldnt be +=?
			tangents[i] += tangent;
			tangents[i+1] += tangent;
			tangents[i+2] += tangent;
		}
	}

	for (GLuint i=0; i<numVertices; ++i)
		tangents[i].Normalise();
}

Vector3 Mesh::GenerateTangent(const Vector3& a, const Vector3& b, const Vector3& c, const Vector2& ta, const Vector2& tb, const Vector2& tc){
	
	Vector2 coord1 = tb-ta;
	Vector2 coord2 = tc-ta;

	Vector3 vertex1 = b-a;
	Vector3 vertex2 = c-a;


	//Work out which local space orientation corresponds to the x-axis in texture space
	Vector3 axis = Vector3(vertex1*coord2.y - vertex2*coord1.y);

	//and determine which way is the positive direction
	float factor = 1.0f / (coord1.x * coord2.y - coord2.x * coord1.y);

	return axis * factor;

}

void Mesh::BufferData(){

	//Bind our vertex Array
	glBindVertexArray(arrayObject);

	//Generate and bind the vertex buffer
	glGenBuffers(1, &bufferObject[VERTEX_BUFFER]);
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject[VERTEX_BUFFER]);

	//Buffer the vertex data generated
	glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(Vector3),
		vertices, GL_STATIC_DRAW);

	//Tell gl that each attribute is 3 floats
	glVertexAttribPointer(VERTEX_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_BUFFER);

	if (textureCoords){
		//Generate and bind a texture buffer (if we have texture coords)
		glGenBuffers(1, &bufferObject[TEXTURE_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[TEXTURE_BUFFER]);

		//Buffer our texture coordinate data
		glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(Vector2),
			textureCoords, GL_STATIC_DRAW);

		//Tell gl that each attribute is 2 floats, and enable the vertex attrib array
		glVertexAttribPointer(TEXTURE_BUFFER, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(TEXTURE_BUFFER);
	}

	if (colours) {
		//Generate and bind a colour buffer (if colours have been set)
		glGenBuffers(1, &bufferObject[COLOUR_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[COLOUR_BUFFER]);

		//Buffer the data to the GPU
		glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(Vector4),
			colours, GL_STATIC_DRAW);

		//Tell openGL that each attribute is 4 floats
		glVertexAttribPointer(COLOUR_BUFFER, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(COLOUR_BUFFER);
	}

	if (indices){
		//Generate an index buffer object and bind it
		glGenBuffers(1, &bufferObject[INDEX_BUFFER]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
			bufferObject[INDEX_BUFFER]);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices*sizeof(GLuint),
			indices, GL_STATIC_DRAW);
	}

	if (normals){
		//Generate and bind the ARRAY buffer to the buffer object
		glGenBuffers(1, &bufferObject[NORMAL_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[NORMAL_BUFFER]);

		//Buffer the data to the GPU
		glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(Vector3),
			normals, GL_STATIC_DRAW);

		//Tell the gpu that each attrib is 3 floats, and enable it
		glVertexAttribPointer(NORMAL_BUFFER, 3, GL_FLOAT, GL_FALSE,0,0);
		glEnableVertexAttribArray(NORMAL_BUFFER);
	}

	if (tangents){

		//Generate and bind the tangent buffer to the vao
		glGenBuffers(1, &bufferObject[TANGENT_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[TANGENT_BUFFER]);

		//Buffer the tangent data
		glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(Vector3),
			tangents, GL_STATIC_DRAW);

		//Tell GL each compenent is 3 floats and enable the vbo
		glVertexAttribPointer(TANGENT_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(TANGENT_BUFFER);
	}

	//Unbind to protect this array object from being modified elsewhere in the
	//program
	glBindVertexArray(0);

}

void Mesh::Draw(){
	//Bind the texture before drawing the mesh
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glActiveTexture(GL_TEXTURE1);	//ALSO BIND THE BUMP MAP TEXTURE
	glBindTexture(GL_TEXTURE_2D, bumpTexture);

	glBindVertexArray(arrayObject);
	//If we are using indices, then use draw elements!
	if (bufferObject[INDEX_BUFFER]){
		glDrawElements(type, numIndices, GL_UNSIGNED_INT, 0);
	} else{
		glDrawArrays(type, 0, numVertices);
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
