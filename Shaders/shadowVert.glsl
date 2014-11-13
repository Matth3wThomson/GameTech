#version 150 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;

//As basic as possible for the shaders as we want this shadow map created quickly!

void main(void){
	gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(position, 1.0);

}