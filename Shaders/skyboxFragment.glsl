#version 150 core

uniform samplerCube cubeTex;
uniform float blend;
uniform vec3 cameraPos;

in Vertex {
	vec3 normal;
} IN;

out vec4 gl_FragColor;

void main(void){
	gl_FragColor = texture(cubeTex, normalize(IN.normal)) * vec4(blend);
	gl_FragColor.a = 1.0;
}