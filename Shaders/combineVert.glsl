#version 150 core

//We only want a simple pass through for the buffer combine shader... as were only rendering a quad!

uniform mat4 projMatrix;

in vec3 position;
in vec2 texCoord;

out Vertex {
	vec2 texCoord;
} OUT;

void main(void){
	gl_Position = projMatrix * vec4(position, 1.0);
	OUT.texCoord = texCoord;
}