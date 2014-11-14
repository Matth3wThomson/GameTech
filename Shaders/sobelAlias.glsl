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

//const float[5] weights = {0.1, 0.2, 0.4, 0.2, 0.1};
const float[4] weights = float[](0.1, 0.2, 0.2, 0.1);
//const float[3] weights = {0.25, 0.5, 0.25};
//const float[9] weights = {0.1, 0.1, 0.1, 0.1, 0.5, 0.1, 0.1, 0.1, 0.1};

//TODO:  Make this actually effective!

void main(void){

	
	// SOBEL OPERATOR (EDGE DETERMINING)
	vec2 LP[9] = vec2[](
						vec2(-pixelSize.x, pixelSize.y), 	vec2(0.0, pixelSize.y), 	vec2(pixelSize.x, pixelSize.y),
						vec2(-pixelSize.x, 0.0),				vec2(0.0, 0.0), 				vec2(pixelSize.x, 0.0),
						vec2(-pixelSize.x, -pixelSize.y),	vec2(0.0, -pixelSize.y), 	vec2(pixelSize.x, -pixelSize.y));
	
	
	// vec4 cols[9];
	float cols[9];
	
	for (int i=0; i<9; i++){
		//cols[i] = texture2D(diffuseTex, IN.texCoord+LP[i]);
		float depth = texture2D(depthTex, IN.texCoord+LP[i]).r;	//RAW DEPTH VALUE FROM depth tex
		cols[i] = (2.0* nearPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane)); //Conversion to linear
	}
	
	gl_FragColor = texture2D(diffuseTex, IN.texCoord) * 0.5;
						
	float magnitude = 	abs( (cols[0] + 2 * cols[1] + cols[2]) - (cols[6] + 2 * cols[7] + cols[8]) ) +
								abs( (cols[2] + 2 * cols[5] + cols[8]) - (cols[0] + 2 * cols[3] + cols[6]) );

								
	if (magnitude > threshold){
		// gl_FragColor = vec4(0,0,0,1);
		// for (int i=0; i<9; i++){
				// gl_FragColor = texture2D(diffuseTex, IN.texCoord + LP[i]);
		// }
		int location = 0;
		for (int i=1; i<8; i+=2){
			gl_FragColor+= texture2D(diffuseTex, IN.texCoord) * weights[location];
			location++;
		}
	} else {
		gl_FragColor = texture2D(diffuseTex, IN.texCoord);
	}
	
}