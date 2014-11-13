#version 150 core

uniform sampler2D diffuseTex;	//This is our scene
uniform sampler2D emissiveTex;	//This is our emissive information
uniform sampler2D specularTex;	//This is our specular information

in Vertex {
	vec2 texCoord;
} IN;

out vec4 gl_FragColor;

void main(void){
	vec3 diffuse = texture(diffuseTex, IN.texCoord).xyz;
	vec3 light = texture(emissiveTex, IN.texCoord).xyz;
	vec3 specular = texture(specularTex, IN.texCoord).xyz;
	
	gl_FragColor.xyz = diffuse * 0.2;	//Ambient lighting
	gl_FragColor.xyz += diffuse * light;	//lambert lighting
	gl_FragColor.xyz += specular;
	
	gl_FragColor.a = 1.0;

}