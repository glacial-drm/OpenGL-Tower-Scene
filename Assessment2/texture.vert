#version 450 core

layout (location = 0) in vec4 vPos;
layout(location =  1) in vec2 vTex;
layout (location = 2) in vec3 vCol;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 col;
out vec2 tex;

void main()
{
	gl_Position = projection * view * model * vPos; // matrix mul right to left, pos mul model then etc
	col = vCol;
	tex = vTex;
}