#version 150 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

in vec3 position;
out mat4 inverseProjView;

void main(void){
	gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(position, 1.0);
	
	//This is necessary to work out the world space of each fragment in the fragment shader
	inverseProjView = inverse(projMatrix * viewMatrix);
}