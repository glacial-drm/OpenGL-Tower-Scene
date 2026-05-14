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

float CalculatePositionalIllumination()
{
	float ambient = 1000; // , ambient = k = 0.1

	// Diffuse calcs
	vec3 Nnor = normalize(nor); 
	vec3 Nto_light = normalize(lightPos - FragPosWorldSpace);  // normalise lightPos - Fragment position in world space -------

	
	float diffuse = max(dot(Nnor, Nto_light), 0);

	// Specular calcs
	vec3 Nfrom_light = -Nto_light; // set equal to negative NToLight
	vec3 NrefLight = reflect(Nfrom_light, Nnor); 
	vec3 camDirection = camPos - FragPosWorldSpace;
	vec3 NcamDirection = normalize(camDirection);

	
	int shininess = 1000;
	float specular = pow(max(dot(NcamDirection, NrefLight), 0), shininess); // result of dot product

	// Attenuation calcs
	float d = length(lightPos - FragPosWorldSpace); // distance()
	float attc = 1.0f, attl = 0.05f, attq = 0.002f;
	
	float attenuation = 1 / (attc + (attl * d) + (attq * pow(d, 2)));

	float phong = (ambient + diffuse + specular) * attenuation;
	return phong;
}

void main()
{
	float phong = CalculatePositionalIllumination();

	fColour = vec4(phong * col * lightColour, 1.f);
}
