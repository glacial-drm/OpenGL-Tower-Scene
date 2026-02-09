#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "error.h"
#include "file.h"
#include "shader.h"


#define NUM_BUFFERS 1
#define NUM_VAOS 1
GLuint Buffers[NUM_BUFFERS];
GLuint VAOs[NUM_VAOS];


float vertices[] =
{
//TL
	//pos					//col (black)	
	-1.0f, 0.0f, 1.0f,  	0.0f, 0.0f, 0.0f,				//v1
	0.f, 1.0f, 0.f,  	    0.0f, 0.0f, 0.0f,				//v2
	-1.0f, 0.f, -1.0f,  	0.0f, 0.0f, 0.0f,				//v3

//TB
	//pos					//col (blue)	
	-1.0f, 0.0f, 1.0f,  	0.f, 1.0f, 0.0f,				//v1
	0.f, 1.0f, 0.f,  	    0.f, 1.0f, 0.0f,				//v2
	1.0f, 0.f, 1.0f,  		0.f, 1.0f, 0.0f,				//v3

//TR
	//pos					//col (green)
	1.0f, 0.0f, 1.0f,  		0.0f, 0.0f, 1.0f,				//v1
	0.f, 1.0f, 0.f,  	    0.0f, 0.0f, 1.0f,				//v2
	1.0f, 0.f, -1.0f,  		0.0f, 0.0f, 1.0f,				//v3

//TF
	//pos					//col (red)
	-1.0f, 0.0f, -1.0f,  	1.f, 0.0f, 0.0f,				//v1
	0.f, 1.0f, 0.f,  	    1.f, 0.0f, 0.0f,				//v2
	1.0f, 0.f, -1.0f,  		1.f, 0.0f, 0.0f,				//v3



//BL
	//pos					//col (grey)
	-1.0f, 0.0f, 1.0f,  	0.5f, 0.5f, 0.5f,				//v1
	0.f, -1.0f, 0.f,  	    0.5f, 0.5f, 0.5f,				//v2
	-1.0f, 0.f, -1.0f,  	0.5f, 0.5f, 0.5f,				//v3

//BB
	//pos					//col (cyan)			
	-1.0f, 0.0f, 1.0f,  	0.f, 1.0f, 1.0f,				//v1
	0.f, -1.0f, 0.f,  	    0.f, 1.0f, 1.0f,				//v2
	1.0f, 0.f, 1.0f,  		0.f, 1.0f, 1.0f,				//v3

//BR
	//pos					//col (magenta)
	1.0f, 0.0f, 1.0f,  		1.0f, 0.0f, 1.0f,				//v1
	0.f, -1.0f, 0.f,  	    1.0f, 0.0f, 1.0f,				//v2
	1.0f, 0.f, -1.0f,  		1.0f, 0.0f, 1.0f,				//v3

//BF
	//pos					//col (gold)
	-1.0f, 0.0f, -1.0f,  	1.f, 1.0f, 0.0f,				//v1
	0.f, -1.0f, 0.f,  	    1.f, 1.0f, 0.0f,				//v2
	1.0f, 0.f, -1.0f,  		1.f, 1.0f, 0.0f,				//v3

};








void ProcessKeyboard(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}


void ResizeCallback(GLFWwindow*, int w, int h)
{
	glViewport(0, 0, w, h);
}


//DECLARE POSITION VARIABLES HERE
glm::vec3 oct_pos = glm::vec3(0.0f, 0.0f, -5.0f); // Stores octahedron positions (using world space coords (-5 away from screen)
glm::vec3 cam_pos = glm::vec3(0.0f, 0.5f, 0.0f); // stores camera/view pos

int main(int argc, char** argv)
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(640, 480, "3D modelling", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, ResizeCallback);

	gl3wInit();

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugCallback, 0);

	unsigned int shaderProgram = CompileShader("triangle.vert", "triangle.frag");



	glCreateBuffers(NUM_BUFFERS, Buffers);
	glGenVertexArrays(NUM_VAOS, VAOs);

	glNamedBufferStorage(Buffers[0], sizeof(vertices), vertices, 0);
	glBindVertexArray(VAOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);



	//ENABLE DEPTH TEST HERE
	glEnable(GL_DEPTH_TEST); // fragments that are hidden are not drawn
		// must clear buffer at start of loop if enabled


	while (!glfwWindowShouldClose(window))
	{
		static const GLfloat bgd[] = { 1.f, 1.f, 1.f, 1.f };
		glClearBufferfv(GL_COLOR, 0, bgd);
		glClear(GL_DEPTH_BUFFER_BIT); // clear buffer

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		glUseProgram(shaderProgram);

		//SET UP AND COPY MODEL MATRIX HERE
		glm::mat4 model = glm::mat4(1.f); // Identity matrix for model
		model = glm::translate(model, oct_pos); // Translate model matrix to octahedron pos
		model = glm::rotate(model, (float)glfwGetTime() / 2, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model)); // copy model matrix to uniform in vert shader

		//SET UP AND COPY VIEW MATRIX HERE
			// Translate vertices of octahedron from world space to view space
		glm::mat4 view = glm::mat4(1.f);
		view = glm::translate(view, -cam_pos);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		//SET UP AND COPY PROJECTION MATRIX HERE
			// create frustum, define projection matrix
				// perspective function includes:
					// fovy - angle of vertical field of view at camera pos
					// aspect - ratio of window width to height
					// zNear - distance of near plane from camera
					// zFar - distance of the far plane from camera
		glm::mat4 projection = glm::mat4(1.f);
		// projection = glm::perspective(glm::radians(45.f), 640.f / 480.f, 1.f, 10.f);
		projection = glm::perspective(glm::radians(25.f), 640.f / 480.f, 1.f, 10.f);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAOs[0]);
		glDrawArrays(GL_TRIANGLES, 0, 8*3);

		glBindVertexArray(0);

		glfwSwapBuffers(window);

		glfwPollEvents();
		ProcessKeyboard(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}