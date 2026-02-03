#version 450 core
	// The above specifies OpenGL version 4.5

layout (location = 0) in vec4 vPos; // input variable of type vec4, vPos, representing vertex position
layout (location = 1) in vec3 vCol; // input var for colour, vec3

out vec3 col; // output variable for colour so fragment shader can parse it from VAO

uniform mat4 transformation;

void main()
{
	gl_Position = transformation * vPos;
	col = vCol;
}

// This vertex shader doesn't change the passed in vertex position, and is called a pass-through shader