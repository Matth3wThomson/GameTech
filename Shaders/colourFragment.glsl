#version 150 core

uniform sampler2D diffuseTex;

in Vertex	{
	vec4 colour;
	vec2 texCoord;
} IN;

out vec4 gl_FragColor;

void main(void)	{
	vec4 value = texture(diffuseTex, IN.texCoord) * IN.colour;
	
	if (value.a == 0.0){ discard; }
	gl_FragColor = value;
}