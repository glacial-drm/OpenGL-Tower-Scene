#pragma once

#include "file.h" // Separate code from lab, but from inference (we need read_file() here)

GLuint CompileShader(const char* vsFilename, const char* fsFilename)
{
	
	// Debug Vars
	int success;
	char infoLog[512];

	// Compiling Shader Process:
		// Use OpenGL to create a shader.
		// Specify the source code. Each shader program is specified as a raw string.
		// Use OpenGL to compile the shader.
	
	// Compile Vertex Shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	char* vertexShaderSource = read_file(vsFilename);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Debugging Vertex Shader
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); // check for compilation error in provided shader, stores result in "*params", success.
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog); // query info log of shader, specifying array for info log output
		fprintf(stderr, "Vertex Shader Compilation Fail - %s\n", infoLog);
	}

	// Compile Fragment Shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	char* fragmentShaderSource = read_file(fsFilename);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Debugging Fragment Shader
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		fprintf(stderr, "Fragment Shader Compilation Fail - %s\n", infoLog);
	}

	// Link Shader (to program) process
		// Use OpenGL to create a shader program
		// Attach the shaders to the program
		// Use OpenGL to link the program
	
	// Link Program
	unsigned int program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	// Debugging Linker
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		fprintf(stderr, "Shader Program Link Fail - %s\n", infoLog);
	}

	// Free memory from read files
	free(fragmentShaderSource);
	free(vertexShaderSource);

	// Delete Shaders and return program
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}
