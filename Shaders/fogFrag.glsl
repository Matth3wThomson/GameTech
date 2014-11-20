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


//TODO: Input a fog colour based on the time of day so that we dont gain lighting at night!
const vec4 fogColour = vec4(0.5, 0.5, 0.5, 1.0);
const float fogFactor = 10.0;

void main(void){
	
	float rawDepth = texture2D(depthTex, IN.texCoord).r;
	
	float depth = (2.0 * nearPlane) / (farPlane + nearPlane - rawDepth * (farPlane - nearPlane));
	
	depth = clamp(depth * fogFactor, 0, 1);
	
	vec4 colour = texture2D(diffuseTex, IN.texCoord);
		
	gl_FragColor = mix(colour, fogColour, depth);
	
}