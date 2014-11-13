#version 150 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

//Input all information required to calculate lighting

out vec4 gl_FragColor[2]; //THIS SHADER OUTPUTS TO TWO COLOUR BUFFERS

void main(void){
	mat3 TBN	=	mat3(IN.tangent, IN.binormal, IN.normal);
	
	vec3 normal = normalize(TBN * (texture2D(bumpTex, IN.texCoord).rgb) * 2.0 - 1.0);
	
	//Output the pass-through values of the objects and their textures to the first colour buffer
	gl_FragColor[0] = texture2D(diffuseTex, IN.texCoord);
	
	//OUTPUT world space normal to the second colour buffer,
	//or "Normal" buffer, to be sampled on the second run
	gl_FragColor[1] = vec4(normal.xyz * 0.5 + 0.5, 1.0);	
}