#version 150 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2DShadow shadowTex;	//PROVIDES SPECIAL FORMS OF TEXTURE SAMPLING!
uniform sampler2D highGroundTex;

uniform int specularPower;
uniform float specFactorMod;

uniform vec4 lightColour;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform float lightRadius;

uniform float highestHeight;

in Vertex {
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj;
} IN;

out vec4 gl_FragColor[3];

void main(void){
	
	mat3 TBN = mat3(IN.tangent, IN.binormal, IN.normal);
	
	vec3 normal = normalize(TBN * (texture2D(bumpTex, IN.texCoord).rgb * 2.0 - 1.0));
	
	float normalizedHeightBlend = clamp(IN.worldPos.y / highestHeight, 0.0, 1.0);
	
	vec4 diffuse = texture2D(diffuseTex, IN.texCoord) * (1 - normalizedHeightBlend);
	diffuse += texture2D(highGroundTex, IN.texCoord) * (0 + normalizedHeightBlend);
	
	
	vec3 incident = normalize(lightPos - IN.worldPos);
	float lambert = max(0.0, dot(incident, normal));
	
	float dist = length(lightPos - IN.worldPos);
	float atten = (1.0 - clamp(dist / lightRadius, 0.0, 1.0));
	
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);
	
	float rFactor = max(0.0, dot(halfDir, normal));
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
	
	gl_FragColor[0] = vec4(colour * atten * lambert, diffuse.a);
	gl_FragColor[0].rgb += (diffuse.rgb * lightColour.rgb) * 0.1;
	

	gl_FragColor[1] = vec4(normal.xyz * 0.5 + 0.5, 1.0);
	gl_FragColor[2] = diffuse;
	
}