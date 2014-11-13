#version 150 core

uniform sampler2D diffuseTex;
uniform vec2 pixelSize;
uniform int pixelOffset;

in Vertex {
	vec2 texCoord;
	vec4 colour;
} IN;

out vec4 gl_FragColor;

void main(void){
	
	//TODO: Make this only double vision in one direction (x axis)
	//DOUBLE VISION
	// gl_FragColor = mix (texture2D(diffuseTex, IN.texCoord) , texture2D(diffuseTex, IN.texCoord + pixelSize.xy * pixelOffset), 0.5);
	gl_FragColor = mix (texture2D(diffuseTex, IN.texCoord) , texture2D(diffuseTex, vec2(IN.texCoord.x + pixelSize.x * pixelOffset, IN.texCoord.y)), 0.5);
	
}