#include <GL/gl3w.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

#include <iostream>

#include "camera.h"
#include "error.h"
#include "file.h"
#include "shader.h"
#include "chunk.h"

#include "do_not_edit.h"


//light direction & position variables here
glm::vec3 lightDirection = glm::vec3(0.1f, -.81f, -.61f);
glm::vec3 lightPos = glm::vec3(2.f, 6.f, 7.f);

SCamera Camera;
bool firstMouse = true; // set to true initially as running program is first time mouse gets input
float lastX = 0.f, lastY = 0.f; // previous position of mouse on screen
float fov = 55.f;

float vertices[] =
{
	//back face
	//pos					//col				//normal
	-0.5f, -0.5f, -0.5f,  	1.f, 1.0f, 1.0f,	0.f, 0.f, -1.f,
	0.5f, -0.5f, -0.5f,  	1.f, 1.0f, 1.0f,	0.f, 0.f, -1.f,
	0.5f,  0.5f, -0.5f,  	1.f, 1.0f, 1.0f,	0.f, 0.f, -1.f,
	0.5f,  0.5f, -0.5f,  	1.f, 1.0f, 1.0f,	0.f, 0.f, -1.f,
	-0.5f,  0.5f, -0.5f,  	1.f, 1.0f, 1.0f,	0.f, 0.f, -1.f,
	-0.5f, -0.5f, -0.5f,  	1.f, 1.0f, 1.0f,	0.f, 0.f, -1.f,

	//front face
	-0.5f, -0.5f,  0.5f,  	1.0f, 1.0f, 1.0f,	0.f, 0.f, 1.f,
	0.5f, -0.5f,  0.5f,  	1.0f, 1.0f, 1.0f,	0.f, 0.f, 1.f,
	0.5f,  0.5f,  0.5f,  	1.0f, 1.0f, 1.0f,	0.f, 0.f, 1.f,
	0.5f,  0.5f,  0.5f,  	1.0f, 1.0f, 1.0f,	0.f, 0.f, 1.f,
	-0.5f,  0.5f,  0.5f,  	1.0f, 1.0f, 1.0f,	0.f, 0.f, 1.f,
	-0.5f, -0.5f,  0.5f,  	1.0f, 1.0f, 1.0f,	0.f, 0.f, 1.f,

	//left face
	-0.5f,  0.5f,  0.5f,  	1.0f, 1.0f, 1.0f,	-1.f, 0.f, 0.f,
	-0.5f,  0.5f, -0.5f,  	1.0f, 1.0f, 1.0f,	-1.f, 0.f, 0.f,
	-0.5f, -0.5f, -0.5f,  	1.0f, 1.0f, 1.0f,	-1.f, 0.f, 0.f,
	-0.5f, -0.5f, -0.5f,  	1.0f, 1.0f, 1.0f,	-1.f, 0.f, 0.f,
	-0.5f, -0.5f,  0.5f,  	1.0f, 1.0f, 1.0f,	-1.f, 0.f, 0.f,
	-0.5f,  0.5f,  0.5f,  	1.0f, 1.0f, 1.0f,	-1.f, 0.f, 0.f,

	//right face
	0.5f,  0.5f,  0.5f,  	1.f, 1.0f, 1.0f,	1.f, 0.f, 0.f,
	0.5f,  0.5f, -0.5f,  	1.f, 1.0f, 1.0f,	1.f, 0.f, 0.f,
	0.5f, -0.5f, -0.5f, 	1.f, 1.0f, 1.0f,	1.f, 0.f, 0.f,
	0.5f, -0.5f, -0.5f,  	1.f, 1.0f, 1.0f,	1.f, 0.f, 0.f,
	0.5f, -0.5f,  0.5f,  	1.f, 1.0f, 1.0f,	1.f, 0.f, 0.f,
	0.5f,  0.5f,  0.5f,  	1.f, 1.0f, 1.0f,	1.f, 0.f, 0.f,

	//bottom face
	-0.5f, -0.5f, -0.5f,  	1.f, 1.0f, 1.0f,	0.f, -1.f, 0.f,
	0.5f, -0.5f, -0.5f,  	1.f, 1.0f, 1.0f,	0.f, -1.f, 0.f,
	0.5f, -0.5f,  0.5f,  	1.f, 1.0f, 1.0f,	0.f, -1.f, 0.f,
	0.5f, -0.5f,  0.5f,  	1.f, 1.0f, 1.0f,	0.f, -1.f, 0.f,
	-0.5f, -0.5f,  0.5f,  	1.f, 1.0f, 1.0f,	0.f, -1.f, 0.f,
	-0.5f, -0.5f, -0.5f,  	1.f, 1.0f, 1.0f,	0.f, -1.f, 0.f,

	//top face
	-0.5f,  0.5f, -0.5f,  	1.0f, 1.f, 1.0f,	0.f, 1.f, 0.f,
	0.5f,  0.5f, -0.5f,  	1.0f, 1.f, 1.0f,	0.f, 1.f, 0.f,
	0.5f,  0.5f,  0.5f,  	1.0f, 1.f, 1.0f,	0.f, 1.f, 0.f,
	0.5f,  0.5f,  0.5f,  	1.0f, 1.f, 1.0f,	0.f, 1.f, 0.f,
	-0.5f,  0.5f,  0.5f,  	1.0f, 1.f, 1.0f,	0.f, 1.f, 0.f,
	-0.5f,  0.5f, -0.5f, 	1.0f, 1.f, 1.0f,	0.f, 1.f, 0.f
};



#define NUM_BUFFERS 1
#define NUM_VAOS 1
GLuint Buffers[NUM_BUFFERS];
GLuint VAOs[NUM_VAOS];

#define WIDTH 1024
#define HEIGHT 768


void drawFloorAndCubes(unsigned int shaderProgram)
{
	glBindVertexArray(VAOs[0]);

	//floor
	glm::mat4 model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(0, -3, 0));
	model = glm::scale(model, glm::vec3(100, 0.1, 100));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glDrawArrays(GL_TRIANGLES, 0, 36);

	//cubes
	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			for (int k = -1; k < 2; k++)
			{
				glm::mat4 model = glm::mat4(1.f);
				model = glm::translate(model, glm::vec3(float(i * 2), float(j * 2), float(k * 2)));
				glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}
	}

}

// CONTAINS SPOTLIGHT CODE -----------------------------
void processKeyboardRotate(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Set light direction & position to point from the current camera front / pos
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		lightDirection = Camera.Front;
		lightPos = Camera.Position;
	}

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
		cameraRotateAroundObject(Camera, glm::vec3(0.f, 0.f, 0.f), cam_dist, x_offset, y_offset);
	}
}


void processKeyboardWalkAround(GLFWwindow* window) // REF: https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/7.4.camera_class/camera_class.cpp
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	float cam_speed = 0.1f * deltaTime;

	// Rotate/Orient the camera in the specified direction (arrow keys)
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		Camera.Position += cam_speed * Camera.Front;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		Camera.Position -= cam_speed * Camera.Front;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		Camera.Position -= glm::normalize(glm::cross(Camera.Front, Camera.Up)) * cam_speed;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		Camera.Position += glm::normalize(glm::cross(Camera.Front, Camera.Up)) * cam_speed;
	}
	
	// Camera.Position.y = 0.f; // force 0 for now 
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) // REF: https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/7.4.camera_class/camera_class.cpp
{
	
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	Camera.Yaw += xoffset;
	Camera.Pitch += yoffset;

	if (Camera.Pitch > 89.0f)
		Camera.Pitch = 89.0f;
	if (Camera.Pitch < -89.0f)
		Camera.Pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
	direction.y = sin(glm::radians(Camera.Pitch));
	direction.z = sin(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
	Camera.Front = glm::normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) // REF: https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/7.4.camera_class/camera_class.cpp
{
	fov -= (float)yoffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
}

void SizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}

vector<float> parse_vertex(vertex v) {
	vector<float> params;

	glm::vec3 pos = v.pos;
	glm::vec3 col = v.col;
	glm::vec3 nor = v.nor;

	params = { pos.x, pos.y, pos.z, col.r, col.g, col.b, nor.x, nor.y, nor.z };
	return params;
}

int main(int argc, char** argv)
{
	glm::vec3 origin = glm::vec3(0.f, 0.f, 0.f); // glm::vec3(-32.f, 0.f, -32.f);
	int originOffset = (dim - 1) * stride; // -1 as dim is array size 32 compared to 0-31 range
	
	int chunksX = 8, chunksZ = 8; // x by 2x breaks, test or do I acc care atp
	int chunkVertTotal = dim * dim * 6; // *6 = *3 (triangle) * 2 (2 triangles per square)
	int vertexTotal = chunkVertTotal * chunksX * chunksZ; // procedural | testVertices // 
	
	int chunkFloatSize = chunkVertTotal * 9; // *9 as 9 components per vert (3 pos, 3 col, 3 nor)
	int floatTotal = chunkFloatSize * chunksX * chunksZ;

	// procedurally generate
		// based on heights ----------------------------------------------------------
			// mountain > grass > water
				// if doing icurrus city then puddles vs deep water
		// trees spawn semi

	
	std::vector<float> testVertsVect;
	float* proceduralVerts = new float[floatTotal];
	
	float* loopVerts = new float[chunkFloatSize]; // single chunk iteration
	
	int gridOffset; 
	chunk** loopChunks = (chunk**) malloc((chunksX * chunksZ) * sizeof(chunk*)); // grid of chunks
	chunk* loopChunk;
	
	std::vector<float> loopVect;
	for (int row = 0; row < chunksX; row++) {
		
		for (int col = 0; col < chunksZ; col++) { // skip on some iterations probably (allow space for non procedural chunks
			
			gridOffset = (row * chunksX) + col;

			loopChunk = perlinChunk(origin);
			loopChunks[gridOffset] = loopChunk;
			
			// adjust the chunk to connect verts
			if (col != 0) { // connect the west verts
				connectTris(loopChunk, loopChunks[gridOffset - 1], 0);
			}
			if (row != 0) { // connect the south verts
				connectTris(loopChunk, loopChunks[gridOffset - chunksX], 1);
			}
			
			// get the verts from chunk and append to vector
			loopVerts = chunkVertices(loopChunk);
			loopVect.assign(loopVerts, loopVerts + chunkFloatSize);

			testVertsVect.insert(testVertsVect.end(), loopVect.begin(), loopVect.end());


			origin.z += originOffset;
		}
		origin.x += originOffset;

		origin.z -= originOffset * chunksZ;
	}
	
	std::copy(testVertsVect.begin(), testVertsVect.end(), proceduralVerts); // this seems to be the issue

	// load models -----------------------------------
		// miku?
		// tower exterior, interior would be a bit crazy
			// if doing interior, make sure to make some parts procedural


	

	// Single chunk
	
	//int vertexTotal = dim * dim * 6; // single chunk procedural
	
	//chunk* testChunk = perlinChunk(glm::vec3(0.f, 0.f, 0.f));
	//float* testVerts = new float[vertexTotal * 9];
	//testVerts = chunkVertices(testChunk);


	// Cornell obj
	
	// int vertexTotal = 96; // cornell box | verts
	
	//const std::string MODEL_PATH = "objs/cornell2/cornell-box.obj";
	//std::vector<triangle> tris;
	//obj_parse(MODEL_PATH.c_str(), &tris);

	//std::vector<float> verts_vect; // create vector then convert to arr

	//for (triangle t : tris) { // index over triangles 

	//	std::vector<float> vert = parse_vertex(t.v1); // parse info we need (pos, col, nor)
	//	verts_vect.insert(verts_vect.end(), vert.begin(), vert.end());

	//	vert = parse_vertex(t.v2);
	//	verts_vect.insert(verts_vect.end(), vert.begin(), vert.end());

	//	vert = parse_vertex(t.v3);
	//	verts_vect.insert(verts_vect.end(), vert.begin(), vert.end());

	//}

	//// get tris and convert to format in vertices[]
	//float* verts = new float[verts_vect.size()];
	//// float verts[864]; // number of vertices * 9
	//std::copy(verts_vect.begin(), verts_vect.end(), verts); // this seems to be working :>

	
	// Lab obj (cubes and plane)

	// int vertexTotal = 36; // lab | vertices

	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Lighting", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, SizeCallback);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	
	gl3wInit();

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugCallback, 0);

	GLuint program = CompileShader("triangle.vert", "triangle.frag");

	InitCamera(Camera);
	cameraWalkAroundSpace(Camera, cam_dist, 0.f, 0.f); // orient camera once on init
	

	glCreateBuffers(NUM_BUFFERS, Buffers);
	glNamedBufferStorage(Buffers[0], sizeof(float) * floatTotal, proceduralVerts, 0);

	glGenVertexArrays(NUM_VAOS, VAOs);
	glBindVertexArray(VAOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);

	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);

	// Colour / UV
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Normals
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glEnable(GL_DEPTH_TEST);


	while (!glfwWindowShouldClose(window))
	{
		static const GLfloat bgd[] = { .8f, .8f, .8f, 1.f };
		glClearBufferfv(GL_COLOR, 0, bgd);
		glClear(GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glUseProgram(program);

		// Copy light direction, light colour, camera position, light position
		glUniform3f(glGetUniformLocation(program, "lightDirection"), lightDirection.x, lightDirection.y, lightDirection.z);
		glUniform3f(glGetUniformLocation(program, "lightColour"), 1.f, 1.f, 1.f);
		glUniform3f(glGetUniformLocation(program, "camPos"), Camera.Position.x, Camera.Position.y, Camera.Position.z);
		glUniform3f(glGetUniformLocation(program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

		glm::mat4 view = glm::mat4(1.f);
		view = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		// Cubes and floor (lab)

		//glm::mat4 projection = glm::mat4(1.f);
		//projection = glm::perspective(glm::radians(45.f), (float)WIDTH / (float)HEIGHT, .01f, 100.f);
		//glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		//drawFloorAndCubes(program);


		// Cornell (self study)

		glm::mat4 projection = glm::mat4(1.f);
		projection = glm::perspective(glm::radians(fov), (float)WIDTH / (float)HEIGHT, .1f, 100.f);
		glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glm::mat4 model = glm::mat4(1.f);
		model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
		model = glm::translate(model, glm::vec3(.1f, -2.5f, 0.f));

		glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		 
		// Bind arrays
		glBindVertexArray(VAOs[0]);
		glDrawArrays(GL_TRIANGLES, 0, vertexTotal);
		glBindVertexArray(0);

		glfwSwapBuffers(window);

		glfwPollEvents();
		// processKeyboardRotate(window);
		processKeyboardWalkAround(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}