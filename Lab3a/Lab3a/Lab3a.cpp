#define _USE_MATH_DEFINES 

#include <iostream>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "error.h"
#include "file.h"
#include "shader.h"

#define DEG2RAD(n) n*(M_PI/180)

#define NUM_BUFFERS 1
#define NUM_VAOS 1
GLuint Buffers[NUM_BUFFERS];
GLuint VAOs[NUM_VAOS];

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

}

void ResizeCallback(GLFWwindow*, int w, int h)
{
	glViewport(0, 0, w, h);
}


#define DEG2RAD(n)	(n)*(M_PI/180)

//DEFINE YOUR FUNCTION FOR CREATING A CIRCLE HERE
float* CreateCircle(int num_segments, float radius)
{
	// float verts[36]; // debugging pointers only shows first element, not whole arr. That's the nature of pointers at work
	float* verts = (float*) malloc(sizeof(float) * num_segments * 3 * 3);
	
	float offset = (float)360 / (float)num_segments;
	float angle = 0;
	
	int j;
	for (int i = 0; i < num_segments; i++) {
		
		j = i * 9;
		
		// v0
		verts[j] = 0.f; // x
		verts[j + 1] = 0.f; // y
		verts[j + 2] = 0.f; // z

		//v1
		verts[j + 3] = sin(DEG2RAD(angle)) * radius;
		verts[j + 4] = cos(DEG2RAD(angle)) * radius;
		verts[j + 5] = 0.f;

		//v2
		verts[j + 6] = sin(DEG2RAD((angle + offset))) * radius; // NEED TO USE BRACKETS, HOVER OVER DEG2RAD WITH AND WITHOUT THEM TO SEE WHY
		verts[j + 7] = cos(DEG2RAD((angle + offset))) * radius;
		verts[j + 8] = 0.f;
		
		angle += offset;
	}

	for (int i = 0; i < (num_segments * 3 * 3); i++) {
		std::cout << verts[i] << std::endl;
	}
	
	return verts;
}

int main()
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(640, 480, "2D modelling", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetWindowSizeCallback(window, ResizeCallback);

	gl3wInit();

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugCallback, 0);

	GLuint program = CompileShader("triangle.vert", "triangle.frag");

	//CREATE CIRCLE HERE
	int num_segs = 32;
	float* verts = CreateCircle(num_segs, 0.5f);

	glCreateBuffers(NUM_BUFFERS, Buffers);

	//COPY VERTICES HERE
	// printf("%lld", sizeof(float) * num_segs * 3 * 3);
	glNamedBufferStorage(Buffers[0], (sizeof(float) * num_segs * 3 * 3), verts, 0);
		// CANNOT USE SIZE OF POINTER FOR ABOVE, ONLY STORES SAME VALUE, PLEASE DON'T MAKE THIS MISTAKE AGAIN

	glGenVertexArrays(NUM_VAOS, VAOs);
	glBindVertexArray(VAOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	
	//SETUP ATTRIBUTES HERE
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(float) * 3), (void*)0);
	glEnableVertexAttribArray(0);

	while (!glfwWindowShouldClose(window))
	{
		static const GLfloat bgd[] = { 1.f, 1.f, 1.f, 1.f };
		glClearBufferfv(GL_COLOR, 0, bgd);

		glUseProgram(program);
		glBindVertexArray(VAOs[0]);
		glDrawArrays(GL_TRIANGLES, 0, num_segs*3);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//FREE THE ALLOCATED MEMORY HERE
	free(verts);

	glfwDestroyWindow(window);
	glfwTerminate();
}