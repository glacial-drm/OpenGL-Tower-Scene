#version 450 core

layout (location = 0) out vec4 fColour;

in vec3 col;
in vec3 nor; // input normal vector ver
in vec3 FragPosWorldSpace; // input frag pos in world space var

// variable names match render loop names
uniform vec3 lightDirection;
uniform vec3 lightPos;
uniform vec3 lightColour;
uniform vec3 camPos;

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

	float phong = ambient + diffuse + specular;
	return phong;
}


void main()
{
	float phong = CalculateDirectionalIllumination();

	fColour = vec4(phong * col * lightColour, 1.f);
}
