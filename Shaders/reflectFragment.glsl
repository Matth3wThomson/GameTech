#version 150 core

uniform sampler2D diffuseTex;
uniform samplerCube cubeTex;	//Automatically handles which of the 6 seperate textures the sample is from

uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;

uniform vec3 cameraPos;

in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 gl_FragColor;

void main(void){

	vec4 diffuse = texture(diffuseTex, IN.texCoord) * IN.colour;
	
	vec3 incident = normalize(IN.worldPos - cameraPos);
	
	float dist = length(lightPos - IN.worldPos);
	float atten = 1.0 - clamp(dist / lightRadius, 0.2, 1.0);
	
	//Gives us the fragment at the vector pointed to, calculated by the reflect function given an incident
	//vector and a normal
	vec4 reflection  = texture(cubeTex, reflect(incident, normalize(IN.normal)));
	
	//Blend it all together!
	gl_FragColor = (lightColour * diffuse * atten) * (diffuse+reflection);
	
}

