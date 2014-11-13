#version 150 core

uniform sampler2D diffuseTex;
uniform vec2 pixelSize;
uniform float threshold;

in Vertex {
	vec2 texCoord;
	vec4 colour;
} IN;

out vec4 gl_FragColor;

//TODO: Make this a passable uniform?
// const float threshold = 0.2;

void main(void){

	
	// SOBEL OPERATOR (EDGE DETERMINING)
	vec2 LP[9] = vec2[](
						vec2(-pixelSize.x, pixelSize.y), 	vec2(0.0, pixelSize.y), 	vec2(pixelSize.x, pixelSize.y),
						vec2(-pixelSize.x, 0.0),				vec2(0.0, 0.0), 				vec2(pixelSize.x, 0.0),
						vec2(-pixelSize.x, -pixelSize.y),	vec2(0.0, -pixelSize.y), 	vec2(pixelSize.x, -pixelSize.y));
	
	
	vec4 cols[9];
	
	for (int i=0; i<9; i++){
		cols[i] = texture2D(diffuseTex, IN.texCoord+LP[i]);
	}
						
	float magnitude = 	abs( (cols[0] + 2 * cols[1] + cols[2]) - (cols[6] + 2 * cols[7] + cols[8]) ) +
								abs( (cols[2] + 2 * cols[5] + cols[8]) - (cols[0] + 2 * cols[3] + cols[6]) );
								
	if (magnitude > threshold){
		gl_FragColor = vec4(0,0,0,1);
	} else {
		gl_FragColor = texture2D(diffuseTex, IN.texCoord);
	}

	
}