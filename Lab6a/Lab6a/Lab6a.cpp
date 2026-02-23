#include <GL/gl3w.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "camera.h"
#include "error.h"
#include "file.h"
#include "shader.h"



float vertices[] =
{
	//pos					//col			
	-0.5f, -0.5f, -0.5f,  	1.f, 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  	1.f, 0.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  	1.f, 0.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  	1.f, 0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  	1.f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  	1.f, 0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  	0.0f, 1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  	0.0f, 1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  	0.0f, 1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  	0.0f, 1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  	0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  	0.0f, 1.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  	0.0f, 0.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  	0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  	0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  	0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  	0.0f, 0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  	0.0f, 0.0f, 1.0f,

	0.5f,  0.5f,  0.5f,  	1.f, 1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  	1.f, 1.0f, 0.0f,
	0.5f, -0.5f, -0.5f, 	1.f, 1.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  	1.f, 1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  	1.f, 1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  	1.f, 1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  	1.f, 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  	1.f, 0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  	1.f, 0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  	1.f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  	1.f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  	1.f, 0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  	0.0f, 1.f, 1.0f,
	0.5f,  0.5f, -0.5f,  	0.0f, 1.f, 1.0f,
	0.5f,  0.5f,  0.5f,  	0.0f, 1.f, 1.0f,
	0.5f,  0.5f,  0.5f,  	0.0f, 1.f, 1.0f,
	-0.5f,  0.5f,  0.5f,  	0.0f, 1.f, 1.0f,
	-0.5f,  0.5f, -0.5f, 	0.0f, 1.f, 1.0f,
};

SCamera Camera; // camera object, moved using enums in camera.h


#define NUM_BUFFERS 1
#define NUM_VAOS 1
GLuint Buffers[NUM_BUFFERS];
GLuint VAOs[NUM_VAOS];

#define WIDTH 640
#define HEIGHT 480



void processKeyboard(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	
	float x_offset = 0.f;
	float y_offset = 0.f;
	bool cam_changed = false;
	// Rotate/Orient the camera in the specified direction (arrow keys)
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		x_offset = 0.f;
		y_offset = 1.f;
		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		x_offset = 0.f;
		y_offset = -1.f;
		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		x_offset = -1.f;
		y_offset = 0.f;
		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) // Offset camera by +1 in the x axis when the right arrow key is preseed
	{
		x_offset = 1.f;
		y_offset = 0.f;
		cam_changed = true;
	}
	
	// Zoom in and out
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		cam_dist -= .1f; // Declared in camera.h
		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		cam_dist += .1f;
		cam_changed = true;
	}


	// Perform relevant changes
	if (cam_changed)
	{
		MoveAndOrientCamera(Camera, glm::vec3(0.f, 0.f, 0.f), cam_dist, x_offset, y_offset);
	}
}

void SizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}

int main(int argc, char** argv)
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Camera", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, SizeCallback);

	gl3wInit();

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugCallback, 0);


	GLuint program = CompileShader("triangle.vert", "triangle.frag");

	InitCamera(Camera); // initialise camera
	MoveAndOrientCamera(Camera, glm::vec3(0.f, 0.f, 0.f), cam_dist, 0.f, 0.f);

	glCreateBuffers(NUM_BUFFERS, Buffers);
	glNamedBufferStorage(Buffers[0], sizeof(vertices), vertices, 0);
	glGenVertexArrays(NUM_VAOS, VAOs);
	glBindVertexArray(VAOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window))
	{
		static const GLfloat bgd[] = { 1.f, 1.f, 1.f, 1.f };
		glClearBufferfv(GL_COLOR, 0, bgd);
		glClear(GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glUseProgram(program);

		glm::mat4 model = glm::mat4(1.f);
		glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		glm::mat4 view = glm::mat4(1.f);
		// view = glm::translate(view, -glm::vec3(0.f, 0.f, 3.f)); // old view matrixformat
		view = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
			// LookAt constructs view matrix from:
				// position - pos of camera in world space
				// "target" - pos of target of the camera
				// "up" - normalised vector defining the camera up direction, helping with orientation
			// This new view matrix format translates all vertices such that the camera is the origin and rotates them relative to "target" and "up"

		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 projection = glm::mat4(1.f);
		projection = glm::perspective(glm::radians(45.f), (float)WIDTH / (float)HEIGHT, .1f, 10.f);
		glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAOs[0]);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glfwSwapBuffers(window);

		glfwPollEvents();
		processKeyboard(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}