#version 150 core

uniform mat4 viewProjMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;


in  vec3 position;
in  vec4 colour;
in vec2 texCoord;

out Vertex {
	vec4 colour;
	vec2 texCoord;
} OUT;

void main(void)	{
	//gl_Position	  = viewProjMatrix * vec4(position, 1.0);
	gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(position, 1.0);
	OUT.colour    = colour;
	OUT.texCoord = texCoord;
}
