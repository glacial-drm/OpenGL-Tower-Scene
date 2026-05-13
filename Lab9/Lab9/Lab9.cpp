#include <GL/gl3w.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "error.h"
#include "file.h"
#include "shader.h"
#include "casteljau.h"
#include "point.h"



#define NUM_BUFFERS 2
#define NUM_VAOS 2
GLuint Buffers[NUM_BUFFERS];
GLuint VAOs[NUM_VAOS];

#define WIDTH 1024
#define HEIGHT 768


void SizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}

void processKeyboard(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main(int argc, char** argv)
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Curves", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, SizeCallback);

	gl3wInit();

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugCallback, 0);

	GLuint program = CompileShader("mvp.vert", "col.frag");

	std::vector<point> ctrl_points;
	ctrl_points.push_back(point(-2.5f, -4.f, 0.f));
	ctrl_points.push_back(point(-5.f, 4.f, 0.f));
	ctrl_points.push_back(point(5.f, -4.f, 0.f));
	ctrl_points.push_back(point(5.f, 4.f, 0.f));
	ctrl_points.push_back(point(2.f, 4.f, 0.f));
	ctrl_points.push_back(point(-2.f, 4.f, 0.f));
	//ctrl_points.push_back(point(0.f, 0.f, 0.f));

	int num_evals = 30;
	std::vector<point> curve = EvaluateBezierCurve(ctrl_points, num_evals);
	
	std::cout << curve.size();

	int num_curve_verts = 0;
	int num_curve_floats = 0;
	float* curve_vertices = MakeFloatsFromVector(curve, num_curve_verts, num_curve_floats, 0.f, 0.f, 0.f);

	int num_ctrl_verts = 0;
	int num_ctrl_floats = 0;
	float* ctrl_vertices = MakeFloatsFromVector(ctrl_points, num_ctrl_verts, num_ctrl_floats, 1.f, 0.f, 0.f);


	glCreateBuffers(NUM_BUFFERS, Buffers);
	glGenVertexArrays(NUM_VAOS, VAOs);

	glBindVertexArray(VAOs[0]);
	glNamedBufferStorage(Buffers[0], sizeof(float) * num_curve_floats, curve_vertices, 0);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(VAOs[1]);
	glNamedBufferStorage(Buffers[1], sizeof(float) * num_ctrl_floats, ctrl_vertices, 0);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	glEnable(GL_DEPTH_TEST);

	glPointSize(8);

	glUseProgram(program);

	while (!glfwWindowShouldClose(window))
	{
		static const GLfloat bgd[] = { 1.f, 1.f, 1.f, 1.f };
		glClearBufferfv(GL_COLOR, 0, bgd);
		glClear(GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glm::mat4 model = glm::mat4(1.f);
		glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		glm::mat4 view = glm::mat4(1.f);
		view = glm::translate(view, -glm::vec3(0.f, 0.f, 12.f));
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)800 / (float)600, .01f, 100.f);
		glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAOs[0]);
		glDrawArrays(GL_LINE_STRIP, 0, num_curve_verts);

		glBindVertexArray(VAOs[1]);
		glDrawArrays(GL_POINTS, 0, num_ctrl_verts);

		glfwSwapBuffers(window);

		glfwPollEvents();
		processKeyboard(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}