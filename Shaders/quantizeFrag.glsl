#version 150 core

uniform sampler2D diffuseTex;
uniform sampler2D depthTex;
uniform vec2 pixelSize;
uniform float threshold;

uniform float nearPlane;
uniform float farPlane;

in Vertex {
	vec2 texCoord;
	vec4 colour;
} IN;

out vec4 gl_FragColor;

const float numShades = 20;

//TODO: Make this a passable uniform?
// const float threshold = 0.2;

void main(void){

	vec4 colour = texture2D(diffuseTex, IN.texCoord);
	
	float intensity = (colour.r + colour.r + colour.r
	+ colour.b + colour.g + colour.g
	+colour.g + colour.g) / 6;
	
	float shadeIntensity = ceil(intensity * numShades) / numShades;
	
	gl_FragColor = colour * shadeIntensity;
	gl_FragColor.a = colour.a;
	
}