#version 150 core

uniform sampler2D diffuseTex;
uniform vec2 pixelSize;
uniform int isVertical;

in Vertex {
	vec2 texCoord;
	vec4 colour;
} IN;

out vec4 gl_FragColor;

//Declare our gaussian blur array
const float weights[5] = float[](0.12, 0.22, 0.32, 0.22, 0.12);

void main(void){


	//BLUR
	vec2 values[5];
	
	if (isVertical == 1){
		values = vec2[](vec2(0.0, -pixelSize.y*3),
			vec2(0.0, -pixelSize.y*2), vec2(0.0, pixelSize.y),	
			vec2(0.0, pixelSize.y*2), vec2(0.0, pixelSize.y*3));
	} else {
		values = vec2[](vec2(-pixelSize.x*3, 0.0),
			vec2(-pixelSize.x*2, 0.0), vec2(pixelSize.x, 0.0),
			vec2(pixelSize.x*2, 0.0), vec2(pixelSize.x*3, 0.0));
	
	}
	
	
	//For each pixel (up 2, current, and down 2) surrounding the current pixel, access their
	//texture coordinate and Add it to the currrent fragment colour based on a weighting defined earlier
	for (int i=0; i<5; i++){
		vec4 tmp = texture2D(diffuseTex, IN.texCoord.xy + values[i]);
		gl_FragColor += tmp * weights[i];
	}
	
}