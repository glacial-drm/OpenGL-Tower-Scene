
#include <iostream>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "file.h"
#include "error.h"

GLfloat vertices[] =
{
	//pos
		// screen space is -1 to 1 x and y, with origin being in the center
		// colours are -1 to 1 for R G and B, with 1 being 255 and -1 being 0
	
	//pos				// col
	0.f,  0.5f,			1.f, 0.f, 0.f,
	-0.5f, -0.5f,		0.f, 1.f, 0.f,
	0.5f,  -0.5f,		0.f, 0.f, 1.f
};



#define NUM_BUFFERS 1
#define NUM_VAOS 1
GLuint Buffers[NUM_BUFFERS];
GLuint VAOs[NUM_VAOS];

float x_offset = 0.f;
float y_offset = 0.f;

void ProcessKeyboard(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		y_offset += 0.01f;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		y_offset -= 0.01f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		x_offset += 0.01f;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		x_offset -= 0.01f;
}

void SizeCallback(GLFWwindow*, int w, int h)
{
	glViewport(0, 0, w, h);
}

int main()
{
	// Make window
	glfwInit(); // init GLFW before calling library methods
	GLFWwindow* window = glfwCreateWindow(640, 480, "A Triangle", NULL, NULL);
		// in the above
			// NULL 1 corresponds to monitor, specifying a sized window instead of fullscreen
			// NULL 2 determines that a window should not share resources
	glfwMakeContextCurrent(window);
		// the above means output is drawn to the passed window
	glfwSetWindowSizeCallback(window, SizeCallback);

	// Init OpenGL
	gl3wInit();
		// we use GL3W which only loads the core profile of OpenGL 3 and 4
			// GLEW is an alternative (OpenGL Extension Wrangler)

	// Specify Vertices
		// done in vertices[]

	// Enable Debugging for OpenGL
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugCallback, 0);


	// Compile Shaders
	GLuint program = CompileShader("triangle.vert", "triangle.frag");

	// Create Buffers. Send to GPU
		// OpenGL uses buffers to store data required for execution in buffer objects.
			// This is a method of interfacing with GPU memory
		// We need to setup two OpenGL objects to specify how the vertex array, vertices[], is parsed and passed to the vertex shader
			// 1. The vertex buffer object (VBO) contains a copy of the vertex arraya
			// 2. The vertex array object (VAO) contains all the detail necessary for OpenGL to render,
				// including a copy of the created VBO and information about how the vertex buffere elements are to be interepreted by the shader.

	glCreateBuffers(NUM_BUFFERS, Buffers); // creates n buffer objects in VRAM, stores hanldes in the buffers arr
	glNamedBufferStorage(Buffers[0], sizeof(vertices), vertices, 0); 
		// The above	
			// Allocates "size" bytes in the buffer object's new data store.
			// Copies data (vertices) from RAM to the VRAM data store to initialise VRAM
			// flags is set to NULL
	glGenVertexArrays(NUM_VAOS, VAOs); // creates n Vertex Array Objects (VAO), stores handles to them in the *arrays arr
	glBindVertexArray(VAOs[0]); // Binds the VAO array so changes can be made specifically to it
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]); // Binds buffer to target (GL_ARRAY_BUFFER)
	// Each vertex in the array is then specified in the VAO using the following pair of lines
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (5 * sizeof(float)), (void*)0);
		// The above
			// Specifies the index attribute has size number of components of the specified type, where each vertex is made from "stride" bytes.
			// offset is the indexo the first element of this attribute after the start of a a particular vertex
		// In the above we specify:
			// The first attribute, index=0, is composed of 2 GL_FLOATs where each vertex is composed of 2 floats
			// The first element has an offset of 0, (void*)0.
	glEnableVertexAttribArray(0); // This enables the passed vertex attribute

	// Additional attributes for passing colour in VAO from vertex shader, so it can be accessed in fragment shader
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (5 * sizeof(float)), (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(1);

	// Render Loop
		// while (window should stay open)
			// clear screen
			// draw
			// swap buffers
			// process input
	while (!glfwWindowShouldClose(window)) // render loop stops when the user closes the passed window
	{
		static const GLfloat bgd[] = { 1.f, 1.f, 1.f, 1.f };
		glClearBufferfv(GL_COLOR, 0, bgd); // Clear buffer to the colour stored in value, bgd.

		glUseProgram(program); // Sets linked shader program to be active, so shaders in program can be used.
		
		// Modify uniforms
			// uniform is a var that can have it's value set by the application program before the GLSL  program is executed
		glUniform1f(glGetUniformLocation(program, "x_offset"), x_offset);
		glUniform1f(glGetUniformLocation(program, "y_offset"), y_offset);
		
		glBindVertexArray(VAOs[0]); // Bind VAO array so vertices can be used for rendering
		glDrawArrays(GL_TRIANGLES, 0, 3);
			// The above	
				// Draws triangles, using GL_TRIANGLES mode, starting from the "first" index, 0, and for "count", 3, vertices
				// This causes GPU to read vertices from the buffer in the bound VAO, and to begin rendering
					// The GPU is not necessarily synchronised with the CPU.
					// Even after this method is executed, there is no guarantee of the GPU having drawn to the colour buffer / starting the rendering process

		// Double buffering is used to improve image quality.
			// A first buffer is used for displaying the colour of the pixels
			// At the same time, a second buffer is written to by OpenGL
			// This allows for buffers to be swapped after a frame has been displayed.
				// The second buffer is displayed while the first buffer is written to.

		glfwSwapBuffers(window); // Swap front and back buffers in double buffering
		
		glfwPollEvents(); // Process input events
		ProcessKeyboard(window);
	}

	// De-Initialisation
	glfwDestroyWindow(window); // Destroy window
	glfwTerminate(); // De-init GLFW
}