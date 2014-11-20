#version 150 core

uniform sampler2D diffuseTex;
uniform sampler2DShadow shadowTex;	//PROVIDES SPECIAL FORMS OF TEXTURE SAMPLING!
uniform int specularPower;
uniform float specFactorMod;

uniform vec4 lightColour;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform float lightRadius;

in Vertex {
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
	vec4 shadowProj;
} IN;

out vec4 gl_FragColor;

void main(void){
	
	
	//vec4 diffuse = texture2D(diffuseTex, IN.texCoord);
	vec4 diffuse = vec4(1,0,0,1);
	
	vec3 incident = normalize(lightPos - IN.worldPos);
	float lambert = max(0.0, dot(incident, IN.normal));
	
	float dist = length(lightPos - IN.worldPos);
	float atten = (1.0 - clamp(dist / lightRadius, 0.0, 1.0));
	
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);
	
	float rFactor = max(0.0, dot(halfDir, IN.normal));
	float sFactor = pow(rFactor, specularPower);
	
	float shadow = 1.0; // 1.0 = no shadowing
	
	//If the vertex in relation to the spot light has a positive w component it is therefore in front of the light.
	//In such a case we sample the shadow map using texture proj...
	if (IN.shadowProj.w > 0.0){
		shadow = textureProj(shadowTex, IN.shadowProj);
	}
	//TEXTURE PROJ INNER WORKINGS
	// Works by performing a perspective divide on the given vector (division by w) then samples the the texture
	//at the x and y positions. It passes back a comparison value between the shadow map and the z value of the vector.
	//Usually between 0 and 1, but on modern hardware shadow filtering can cause this value to be between the extremes
	
	lambert *= shadow;
	
	vec3 colour = (diffuse.rgb * lightColour.rgb);
	colour += (lightColour.rgb * sFactor) * specFactorMod;
	
	gl_FragColor = vec4(colour * atten * lambert, diffuse.a);
	gl_FragColor.rgb += (diffuse.rgb * lightColour.rgb) * 0.1;
	
	//gl_FragColor = vec4(1,1,1,1);
	
	
	//gl_FragColor.rgb = IN.tangent;
	//gl_FragColor.rgb = normal;
	
	// gl_FragColor = IN.shadowProj;
	//gl_FragColor = IN.shadowProj;
	
}