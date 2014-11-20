#version 150 core

uniform sampler2D depthTex;
uniform sampler2D normTex;

uniform vec2 pixelSize;
uniform vec3 cameraPos;

uniform float lightRadius;
uniform vec3 lightPos;
uniform vec4 lightColour;

in mat4 inverseProjView;

out vec4 gl_FragColor[2];	//Diffuse texture and specular colour


//TODO: Possibly using a shadow tex in this shader we could determine deferred shadows?

void main(void){
	//This tells us the screenspace coordinate of our current fragment being rendered. We convert this 
	//to screen space coordinate so we can use it sample our textures
	vec3 pos = vec3((gl_FragCoord.x * pixelSize.x), (gl_FragCoord.y * pixelSize.y), 0.0);
	
	//We get our z value from the depth texture for our world pos variable
	pos.z	= texture(depthTex, pos.xy).r;
	
	
	//We sample the texture using the found coordinates and convert the value found to
	//clip space. 
	vec3 normal = normalize((texture(normTex, pos.xy).xyz * 2.0) - 1.0);
	
	//We can then convert the value of position to world space
	vec4 clip = inverseProjView * vec4(pos * 2.0 - 1.0, 1.0);
	
	//Dont forget division by w!
	pos = clip.xyz / clip.w;
	
	float dist = length(lightPos - pos);
	float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);
	
	if (atten == 0.0){
		discard;	//IF THIS FRAGMENT ISNT in range of a given light, then dont keep drawing it!
	}
	
	 vec3 incident	=	normalize(lightPos - pos);
	//vec3 incident	=	normalize(pos - lightPos);
	vec3 viewDir = normalize(cameraPos - pos);
	vec3 halfDir = normalize(incident + viewDir);
	
	float lambert = clamp(dot(incident, normal), 0.0, 1.0);
	float rFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
	float sFactor = pow(rFactor, 33.0);
	
	gl_FragColor[0] = vec4(lightColour.xyz * lambert * atten, 1.0);
	gl_FragColor[1] = vec4(lightColour.xyz * sFactor * atten * 0.33, 1.0);
}