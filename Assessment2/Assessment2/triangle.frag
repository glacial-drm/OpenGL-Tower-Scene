#version 450 core

layout (location = 0) out vec4 fColour;

// in vec3 col;
in vec2 tex;

uniform sampler2D Texture;

void main()
{
	// fColour = vec4(col, 1.0f); // TRIANGLE COLOUR
	// fColour = vec4(.0f, .0f, .0f, 1.0f); // TRIANGLE COLOUR
	fColour =  texture(Texture, tex); // performs sampling for this fragment
}