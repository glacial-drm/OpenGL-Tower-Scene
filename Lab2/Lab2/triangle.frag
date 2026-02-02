#version 450 core
	// The above specifies OpenGL version 4.5

layout (location = 0) out vec4 fColour; // Output variable responsible for fragment colour

in vec3 col; // input variable for colour from VAO

void main()
{
	fColour = vec4(col,  1.0f); // Output variable set to colour col, meaning all fragments passed into this shader have the colour col
}