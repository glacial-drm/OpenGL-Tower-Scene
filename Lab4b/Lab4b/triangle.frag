#version 450 core

layout (location = 0) out vec4 fColour;

in vec3 col;

uniform vec3 uCol;

void main()
{
	fColour = vec4(uCol, 1.0f); // TRIANGLE COLOUR
}