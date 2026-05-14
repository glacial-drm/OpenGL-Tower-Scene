#version 450 core

layout (location = 0) out vec4 fColour;

in vec3 col;
in vec3 nor; // input normal vector ver
in vec3 FragPosWorldSpace; // input frag pos in world space var
in vec4 FragPosProjectedLightSpace;

uniform sampler2D shadowMap;

// variable names match render loop names
uniform vec3 lightDirection;
uniform vec3 lightPos;
uniform vec3 lightColour;
uniform vec3 camPos;

float shadowOnFragment(vec4 FragPosProjectedLightSpace)
{
	// shadow map is a copy of light space z-buffer, e.g. from pov of light source
	// OpenGL normalises projected light space and transforms resultant NDC space to screen space
	// frag shader contains frag pos in projected light space as input var
		// we compare z-value of this frag, e.g. dist to light, with dist to fragment closest to light source (one that is hence illuminated and stored in depth map)

	// normalise projected light space fragment pos and transform to screen space
	vec3 ndc = FragPosProjectedLightSpace.xyz / FragPosProjectedLightSpace.w;
	vec3 ss = (ndc+1)*0.5;

	// get dist of fragment to light directly from z-value of fragment in screen space
	float fragDepth = ss.z;
	
	// get dist of frag closest to light source, and hence illuminated from shadow map, using x and y coords of fragment in projected light space and OpenGl texture sampling 
	// hence we only need red component from texel
	float litDepth = texture(shadowMap, ss.xy).r;

	// create bias to resolve shadow acne (also add to shadow below)
	vec3 Nnor = normalize(nor);
	vec3 Ntolight = normalize(-lightDirection);
	float bias = max(0.04 * (1.0 - dot(Nnor, Ntolight)), 0.005);
	
	float shadow = 0.f;
	// fgragment is in shadow if fragment depth is greater than dist to fragment closest to light source
	shadow = fragDepth > (litDepth+bias) ? 1.0 : 0.0;

	if (fragDepth > 1) // fix far distance shadow mess by making forcing fragments outside view volume to be illuminated
		shadow = 0.f;

	return shadow;
}

float CalculateDirectionalIllumination()
{
	float ambient = 0.1; // , ambient = k = 0.1

	// Diffuse calcs
	vec3 Nnor = normalize(nor); // normalise surface normal input 
	vec3 Nto_light = -normalize(lightDirection); // Negate and Normalise light direction uniform

	float diffuse = max(dot(Nnor, Nto_light), 0); // Perform dot product Nnor with Nto_light, and ensure the result is positive

	// Specular calcs
	vec3 Nfrom_light = normalize(lightDirection); // Normalise light direction uniform
	vec3 NrefLight = reflect(Nfrom_light, Nnor); // Reflect Nfrom_light on Nnor
	vec3 camDirection = camPos - FragPosWorldSpace; // camera position uniform - fragment position in world space input variable
	vec3 NcamDirection = normalize(camDirection); // normalize camDirection

	int shininess = 128;
	float specular = pow(max(dot(NcamDirection, NrefLight), 0), shininess); // Perform dot product of NcamDirection with rLight, esnure result is positive, raise to the power of shininess
	
	float shadow = shadowOnFragment(FragPosProjectedLightSpace);
	float phong = ambient + ( (1.f - shadow) * (diffuse + specular) );
	return phong;
}


void main()
{
	float phong = CalculateDirectionalIllumination();

	fColour = vec4(phong * col * lightColour, 1.f);
}
