#version 450 core

layout (location = 0) in vec4 vPos;

uniform mat4 projectedLightSpaceMatrix;
uniform mat4 model;

void main()
{
	// transform vertices to world space, then projected light space
	gl_Position = projectedLightSpaceMatrix * model * vPos;
}