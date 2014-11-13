#version 150 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;

in Vertex {
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 gl_FragColor;

void main(void){

	
	//Base diffuse colour of the fragment
	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	
	//Create TBN matrix to convert tangent space normal into world space
	mat3 TBN = mat3(IN.tangent, IN.binormal, IN.normal);
	
	//Transform the vec3 derived from the bump map to give the world space normal. 
	// multiplication by 2 and subtraction by 1 is to convert from texture space to clip space!
	vec3 normal = normalize(TBN * (texture(bumpTex, IN.texCoord).rgb * 2.0 - 1.0));
	
	//Work out the incident vector beween the currently interpolated fragment world pos
	//and current lights world pos, normalized
	vec3 incident = normalize(lightPos - IN.worldPos);
	
	//Then work out the LAMBERTIAN reflectance for the fragment by performing a dot product
	//operation on the incident and normal calculate from bump map. Clamped to 0 to prevent negative colours.
	float lambert = max(0.0, dot(incident, normal));
	
	//Calculate the distance of the current fragment from the light (FOR ATTENUATION)
	float dist = length(lightPos - IN.worldPos);
	
	float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);
	
	//Calculate the vector between the fragment and the camera position
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	
	//Calculate the half angle between the view direction and the incident vector (angle the light source comes from)
	vec3 halfDir = normalize(incident + viewDir);
	
	//Calculate the specular reflection of the current fragment (again clamped to zero) based on the bump map
	float rFactor = max(0.0, dot(halfDir, normal));
	
	//Then raise the specularity value to the power 50 to represent how shiny our object is. (50 COULD BE A UNIFORM or GLOSS MAPPING)
	float sFactor = pow(rFactor, 1.0);
	
	//Calculate the colour of the fragment with the light colour affecting it
	vec3 colour = (diffuse.rgb * lightColour.rgb);
	
	//Add specularity to the fragment
	colour += (lightColour.rgb * sFactor) * 0.01;
	
	//Create the colour based on the final colour attenuation and lambert values
	gl_FragColor = vec4(colour * atten * lambert, diffuse.a);
	
	//Add some ambient lighting
	gl_FragColor.rgb += (diffuse.rgb * lightColour.rgb) * 0.1;
	
	
}