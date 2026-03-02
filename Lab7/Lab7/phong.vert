#version 450 core

layout(location = 0) in vec4 vPos;
layout(location = 1) in vec3 vCol;
layout(location = 2) in vec3 vNor; // Normal input vector

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 col;
out vec3 nor; // Normal output vector
out vec3 FragPosWorldSpace; // Output fragment position in world space -> calculate the illumination

void main()
{
	gl_Position = projection * view * model * vPos;
	col = vCol;
	nor = mat3(transpose(inverse(model))) * vNor; // transpose() and inverse() esnure normal remains perpendicular to the surface
	FragPosWorldSpace = vec3(model * vPos); // Transform vertex position to world space
}
