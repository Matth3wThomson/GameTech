#version 150 core

 uniform sampler2D diffuseTex;
uniform vec2 pixelSize;

in Vertex {
	vec2 texCoord;
	vec4 colour;
} IN;

// out vec4 gl_FragColor;

// //Declare our gaussian blur array
// //THIS IS JUST BLUR, BUT WE ADD COLOURS TO A VALUE GREATER THAN 1
// const float weights[5] = float[](0.01, 0.05, 1.0, 0.05, 0.01);

// void main(void){

	// float luminance;
	
	// gl_FragColor = texture2D(diffuseTex, IN.texCoord);
	
	// //BLOOM
	// vec2 values[5];
	
	// if (isVertical == 1){
		// values = vec2[](vec2(0.0, -pixelSize.y*3),
			// vec2(0.0, -pixelSize.y*2), vec2(0.0, pixelSize.y),	
			// vec2(0.0, pixelSize.y*2), vec2(0.0, pixelSize.y*3));
	// } else {
		// values = vec2[](vec2(-pixelSize.x*3, 0.0),
			// vec2(-pixelSize.x*2, 0.0), vec2(pixelSize.x, 0.0),
			// vec2(pixelSize.x*2, 0.0), vec2(pixelSize.x*3, 0.0));
	// }
	
	// //For each pixel (up 2, current, and down 2) surrounding the current pixel, access their
	// //texture coordinate and Add it to the currrent fragment colour based on a weighting defined earlier
	// vec4 addColour;
	// int coloursSampled;
	
	// for (int i=0; i<5; i++){
		// if (i != 3){
			// vec4 tmp = texture2D(diffuseTex, IN.texCoord.xy + values[i]);
			// luminance = (tmp.r+tmp.r+tmp.b+tmp.g+tmp.g+tmp.g)/6;
			
			// if (luminance > 0.6){
				// addColour += tmp * weights[i];
				// coloursSampled++;
			// }
		// }
	// }
	
	// addColour /= coloursSampled;
	// gl_FragColor += addColour;
		
	
// }

//uniform sampler2D bgl_RenderedTexture;

out vec4 gl_FragColor;

const float pixelGap = 1.0f;
const int pixelSampleX = 4;
const int pixelSampleY = 3;

void main()
{
   vec4 sum = vec4(0);
   int j;
   int i;

   for( i= -pixelSampleX ;i < pixelSampleX; i++)
   {
        for (j = -pixelSampleY; j < pixelSampleY; j++)
        {
            sum += texture2D(diffuseTex, IN.texCoord + vec2(j, i)*pixelSize*pixelGap) * 0.25;
        }
   }
   
   vec4 colour = texture2D(diffuseTex, IN.texCoord);
   
    if (colour.r < 0.3){
       gl_FragColor = sum*sum*0.012 + colour;
	   
    } else {
        if (colour.r < 0.5){
            gl_FragColor = sum*sum*0.009 + colour;
			
        } else {
            gl_FragColor = sum*sum*0.0075 + colour;
        }
    }
}