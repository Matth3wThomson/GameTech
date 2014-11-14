#version 150 core

 uniform sampler2D diffuseTex;
uniform vec2 pixelSize;

in Vertex {
	vec2 texCoord;
	vec4 colour;
} IN;

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
   
   float luminance = 0.2126 * colour.r + 0.75152 * colour.g + 0.0722 * colour.b;
   
   if (luminance < 0.3){
		gl_FragColor = sum*sum*0.012 + colour;
		
   } else {
		if (luminance < 0.5){
			gl_FragColor = sum*sum*0.009 + colour;
		} else {
			gl_FragColor = sum*sum*0.0075 + colour;
		}
	}
   
    // if (colour.r < 0.3){
       // gl_FragColor = sum*sum*0.012 + colour;
	   
    // } else {
        // if (colour.r < 0.5){
            // gl_FragColor = sum*sum*0.009 + colour;
			
        // } else {
            // gl_FragColor = sum*sum*0.0075 + colour;
        // }
    // }
}