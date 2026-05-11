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
#include "casteljau.h"
#include "point.h"

// #include "texture.h"
#include "shadow.h"

#include "obj-load.h"
// #include "do_not_edit.h"

#define STB_IMAGE_IMPLEMENTATION // lines 4-13 in "stb_image.h"
#include "stb_image.h"

//light direction & position variables here
glm::vec3 lightDirection = glm::vec3(0.1f, -.81f, -.61f);
glm::vec3 lightPos = glm::vec3(2.f, 6.f, 7.f);

SCamera Camera;
bool firstMouse = true; // set to true initially as running program is first time mouse gets input
float lastX = 0.f, lastY = 0.f; // previous position of mouse on screen
float fov = 55.f;

#define NUM_BUFFERS 1
#define NUM_VAOS 1
GLuint Buffers[NUM_BUFFERS];
GLuint VAOs[NUM_VAOS];

#define WIDTH 1024
#define HEIGHT 768
#define SH_MAP_WIDTH 2048
#define SH_MAP_HEIGHT 2048

float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};
vector<std::string> faces
{
	"resources/textures/skybox/right.jpg",
	"resources/textures/skybox/left.jpg",
	"resources/textures/skybox/top.jpg",
	"resources/textures/skybox/bottom.jpg",
	"resources/textures/skybox/front.jpg",
	"resources/textures/skybox/back.jpg"
};

// Lab Code (remove) -----------------------------
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

// Camera 
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

	float cam_speed = 3.f * deltaTime; //0.1f default

	//// Set light direction & position to point from the current camera front / pos
	//if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	//{
	//	lightDirection = Camera.Front;
	//	lightPos = Camera.Position;
	//}

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
	
	// Camera.Position.y = 0.f; // force 0 if need be --------------------
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

	float sensitivity = 0.5f;
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

// Window Size
void SizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}


// Obj Loading
vector<float> parse_vertex(vertex v, bool isTex, bool alpha) {
	vector<float> params;

	glm::vec3 pos = v.pos;
	glm::vec3 col = v.col;
	glm::vec3 nor = v.nor;
	glm::vec2 tex = v.tex;
	float alp = v.alpha;

	params = { pos.x, pos.y, pos.z, col.r, col.g, col.b, nor.x, nor.y, nor.z };
	
	if (isTex) {
		params = { pos.x, pos.y, pos.z, col.r, col.g, col.b, tex.x, tex.y};
		// params = { pos.x, pos.y, pos.z, nor.x, nor.y, nor.z, tex.x, tex.y }; // passing normals instead of colour
	}

	if (alpha) {
		params.push_back(alp);
	}
	
	return params;
}

float* load_model(const std::string MODEL_PATH, int &vertexTotalOutput, bool isTex, bool alpha) {
	
	// bool isTex: either texture or colour
	// bool alpha: alpha values

	std::vector<float> verts_vect;

	std::vector<triangle> tris;
	obj_parse(MODEL_PATH.c_str(), &tris);

	for (triangle t : tris) { // index over triangles 

		std::vector<float> vert = parse_vertex(t.v1, isTex, alpha); // parse info we need (pos, col, nor)
		verts_vect.insert(verts_vect.end(), vert.begin(), vert.end());

		vert = parse_vertex(t.v2, isTex, alpha);
		verts_vect.insert(verts_vect.end(), vert.begin(), vert.end());

		vert = parse_vertex(t.v3, isTex, alpha);
		verts_vect.insert(verts_vect.end(), vert.begin(), vert.end());

	}

	// get tris and convert to format in vertices[]
	vertexTotalOutput = verts_vect.size();
	float* verts = new float[vertexTotalOutput];
	std::copy(verts_vect.begin(), verts_vect.end(), verts); // this seems to be working :>

	return verts;
}

// Load textures: REF LEARNOPENGL ---------------------
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D); // ALSO USE OF MIPMAPS HERE -------------------------

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// Skybox: REF LEARNOPENGL ---------------------
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

// Shadow ------------------------------------------------------ wouldn't this apply to all the geometry.....
void generateDepthMap(unsigned int shadowShaderProgram, ShadowStruct shadow, glm::mat4 projectedLightSpaceMatrix)
{

	glViewport(0, 0, SH_MAP_WIDTH, SH_MAP_HEIGHT);


	glBindFramebuffer(GL_FRAMEBUFFER, shadow.FBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Render scene using model matrices
	glUseProgram(shadowShaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "projectedLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(projectedLightSpaceMatrix));
	
	drawFloorAndCubes(shadowShaderProgram); // draw and write z-values to framebuffer in shadow structure ------------------------------------- draw smth else

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderWithShadow(unsigned int renderShaderProgram, ShadowStruct shadow, glm::mat4 projectedLightSpaceMatrix)
{
	glViewport(0, 0, WIDTH, HEIGHT); // window dimensions are different to shadow map dimensions, set viewport transformation back to using window dimensions

	// clear colour buffer
	static const GLfloat bgd[] = { .8f, .8f, .8f, 1.f };
	glClearBufferfv(GL_COLOR, 0, bgd);
	glClear(GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// activate shadow renderShaderProgram
	glUseProgram(renderShaderProgram);

	glBindTexture(GL_TEXTURE_2D, shadow.Texture); // bind texture being used to store depth map

	// pass the light and camera attributes to the uniforms
	glUniformMatrix4fv(glGetUniformLocation(renderShaderProgram, "projectedLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(projectedLightSpaceMatrix)); // load projected light space matrix to shaders as uniform
	glUniform3f(glGetUniformLocation(renderShaderProgram, "lightDirection"), lightDirection.x, lightDirection.y, lightDirection.z);
	glUniform3f(glGetUniformLocation(renderShaderProgram, "lightColour"), 0.f, 0.f, 1.f);
	glUniform3f(glGetUniformLocation(renderShaderProgram, "camPos"), Camera.Position.x, Camera.Position.y, Camera.Position.z);
	glUniform3f(glGetUniformLocation(renderShaderProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	// set up the view and projection matrices ------------------------------------------------------------------- perhaps edit
	glm::mat4 view = glm::mat4(1.f);
	view = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
	glUniformMatrix4fv(glGetUniformLocation(renderShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection = glm::mat4(1.f);
	projection = glm::perspective(glm::radians(45.f), (float)WIDTH / (float)HEIGHT, .01f, 100.f);
	glUniformMatrix4fv(glGetUniformLocation(renderShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// draw for real ------------------------------------------------------------------- draw smth else
	drawFloorAndCubes(renderShaderProgram);
}


int main(int argc, char** argv)
{
	// PRESET CHUNKS

	glm::vec3 origin = glm::vec3(-32.f, 0.f, -32.f); // glm::vec3(0.f, 0.f, 0.f); 
	int originOffset = (dim - 1) * stride; // -1 as dim is array size 32 compared to 0-31 range
	
	int chunksX = 12, chunksZ = 10; // corresponds to maxHeights arr
	int chunkVertTotal = dim * dim * 6; // *6 = *3 (triangle) * 2 (2 triangles per square)
	int vertexTotal = chunkVertTotal * chunksX * chunksZ; // procedural | testVertices // 
	
	int vertexFloatTotal = 8; // 9 for colour (3 components) // 8 for texture (2 components)
	int chunkFloatSize = chunkVertTotal * vertexFloatTotal; // *9 as 9 components per vert (3 pos, 3 col, 3 nor)
	int floatTotal = chunkFloatSize * chunksX * chunksZ;

	std::vector<float> testVertsVect;
	float* proceduralVerts = new float[floatTotal];
	
	float* loopVerts = new float[chunkFloatSize]; // single chunk iteration
	
	int gridOffset; 
	chunk** loopChunks = (chunk**) malloc((chunksX * chunksZ) * sizeof(chunk*)); // grid of chunks
	chunk* loopChunk;
	
	std::vector<float> loopVect;

	int height = 20;
	int maxHeights[12][10]{
		{200, 200, 200, 200, 200, 200, 200, 200, 200, 200},
		{200, -10, -10, 20, 20, 20, 20, -10, -10, 200},
		{200, -10, -10, 20, 20, 20, 20, -10, -10, 200},
		{200, -10, -10, 20, 20, 20, 20, -10, -10, 200},
		{200, -10, -10, 20, 20, 20, 20, -10, -10, 200},
		{200, -10, -10, -10, 20, 20, -10, -10, -10, 200},
		{200, -10, -10, -10, 20, 20, -10, -10, -10, 200},
		{200, 200, 20, 20, 20, 20, 20, 20, 20, 200},
		{200, 200, 2, 2, 20, 20, 2, 2, 2, 200},
		{200, 200, 2, 2, 2, 2, 2, 2, 2, 200},
		{200, 200, 2, 2, 2, 2, 2, 2, 2, 200},
		{200, 200, 200, 200, 200, 200, 200, 200, 200, 200}
	};

	// SELECT TEXTURE BASED ON CHUNK -------------------------------------

	for (int row = 0; row < chunksX; row++) {
		
		for (int col = 0; col < chunksZ; col++) { // skip on some iterations probably (allow space for non procedural chunks
			
			gridOffset = (row * chunksX) + col;

			if (col > 1) {
				loopChunk = createChunk(origin, maxHeights[(chunksX - 1) - row][col], true); // chunksX - row to account for 2d array indexing (0,0 top left) vs chunk indexing (0,0 bottom left)
			}
			else {
				loopChunk = createChunk(origin, maxHeights[(chunksX - 1) - row][col], true);
			}

			loopChunks[gridOffset] = loopChunk;
			
			// adjust the chunk to connect verts
			if (col != 0) { // connect the west verts
				connectTris(loopChunk, loopChunks[gridOffset - 1], 0);
			}
			if (row != 0) { // connect the south verts
				connectTris(loopChunk, loopChunks[gridOffset - chunksX], 1);
			}
			
			// get the verts from chunk and append to vector
			loopVerts = chunkVertices(loopChunk, vertexFloatTotal);
			loopVect.assign(loopVerts, loopVerts + chunkFloatSize);

			testVertsVect.insert(testVertsVect.end(), loopVect.begin(), loopVect.end());


			origin.z += originOffset;
		}
		origin.x += originOffset;

		origin.z -= originOffset * chunksZ;
	}
	
	std::copy(testVertsVect.begin(), testVertsVect.end(), proceduralVerts);

	// Tower floor chunk(s)
		// single repeating using different y values

	int alphaVertexTotal = dim * dim * 6 * 7; // single chunk procedural
	glm::vec3 towerOrigin = glm::vec3(0.f, 0.f, 0.f);// glm::vec3(-6300.f, 80.f + 2000.f , -3500.f);
	chunk* alphaChunk = createChunk(towerOrigin, 2, true); // change origin back once done ---------------------------------
	// float* alphaVerts = new float[vertexTotal * 10]; // * 10 for alpha // now 7 for alpha
	// alphaVerts = chunkVertices(alphaChunk);
	float* alphaVerts  = chunkVertices(alphaChunk, 7);
	
	
	// PROCEDURALLY GENERATED CHUNKS ----------------------------------------------
		// surround the above 
		
		// trees spawn semi
		
			




	// load models -----------------------------------
		// HOUSE 1
		// CAVE
		// BIRD
		// CLOUD MODEL
			// surrounding tower
		// Miku

		// miku?
		// tower exterior, interior would be a bit crazy
			// if doing interior, make sure to make some parts procedural
	
	int towerVertsTotal; // get material file working
	float* towerVerts = load_model("resources/objs/tower.obj", towerVertsTotal, true, false);

	int tornadoVertsTotal; // get material file working
	float* tornadoVerts = load_model("resources/objs/tornado.obj", tornadoVertsTotal, true, false);

	int eagleVertsTotal; // get material file working
	float* eagleVerts = load_model("resources/objs/eagle.obj", eagleVertsTotal, true, false);

	int bellVertsTotal; // get material file working
	float* bellVerts = load_model("resources/objs/bell.obj", bellVertsTotal, false, false);



	// BEZIER CURVE ----------------------------------
	std::vector<point> ctrl_points;
	ctrl_points.push_back(point(-5.f, -4.f, 0.f));
	ctrl_points.push_back(point(-5.f, 4.f, 0.f));
	ctrl_points.push_back(point(5.f, -4.f, 0.f));
	ctrl_points.push_back(point(5.f, 4.f, 0.f));

	int num_evals = 30;
	std::vector<point> curve = EvaluateBezierCurve(ctrl_points, num_evals);

	int num_curve_verts = 0;
	int num_curve_floats = 0;
	float* curve_vertices = MakeFloatsFromVector(curve, num_curve_verts, num_curve_floats, 0.f, 0.f, 0.f);

	int num_ctrl_verts = 0;
	int num_ctrl_floats = 0;
	float* ctrl_vertices = MakeFloatsFromVector(ctrl_points, num_ctrl_verts, num_ctrl_floats, 1.f, 0.f, 0.f);



	glfwInit();

	glfwWindowHint(GLFW_SAMPLES, 8); // 8x multisampling anti aliasing
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Lighting", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, SizeCallback);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	
	gl3wInit();

	//// Get the max number of samples for multisampling
	//int max_samples;
	//glGetIntegerv(GL_MAX_SAMPLES, &max_samples); // store queried enum in max_samples, guaranteed to be at least 4
	//printf("max samples supported = %d\n", max_samples);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugCallback, 0);

	GLuint triangleProgram = CompileShader("triangle.vert", "triangle.frag");

	GLuint cubemapProgram = CompileShader("skybox.vert", "skybox.frag");
	GLuint alphaProgram = CompileShader("alpha.vert", "alpha.frag");
	GLuint reflectionProgram = CompileShader("reflection.vert", "reflection.frag");
	GLuint refractionProgram = CompileShader("reflection.vert", "refraction.frag");


	GLuint textureProgram = CompileShader("texture.vert", "texture.frag"); //
	
	GLuint phongProgram = CompileShader("phong.vert", "phong.frag"); // MAKE MORE VERSIONS FOR EACH INDIVIDUAL TYPE OF LIGHT WITHIN PHONG FRAG ------------------------------------------
	GLuint shadowProgram = CompileShader("shadow.vert", "shadow.frag");

	InitCamera(Camera);





	// PER OBJ BASIS
		// TOWER -> PILLARS -> could pre render
	unsigned int towerVAO, towerVBO;
	glGenVertexArrays(1, &towerVAO);
	glGenBuffers(1, &towerVBO);
	glBindVertexArray(towerVAO);
	glBindBuffer(GL_ARRAY_BUFFER, towerVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * towerVertsTotal, towerVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 * sizeof(float)), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	
	unsigned int tornadoVAO, tornadoVBO;
	glGenVertexArrays(1, &tornadoVAO);
	glGenBuffers(1, &tornadoVBO);
	glBindVertexArray(tornadoVAO);
	glBindBuffer(GL_ARRAY_BUFFER, tornadoVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tornadoVertsTotal, tornadoVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 * sizeof(float)), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int eagleVAO, eagleVBO;
	glGenVertexArrays(1, &eagleVAO);
	glGenBuffers(1, &eagleVBO);
	glBindVertexArray(eagleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, eagleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * eagleVertsTotal, eagleVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 * sizeof(float)), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	
	unsigned int bellVAO, bellVBO;
	glGenVertexArrays(1, &bellVAO);
	glGenBuffers(1, &bellVBO);
	glBindVertexArray(bellVAO);
	glBindBuffer(GL_ARRAY_BUFFER, bellVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bellVertsTotal, bellVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// PRESET CHUNKS
	unsigned int procVBO, procVAO;
	glGenVertexArrays(NUM_VAOS, &procVAO);
	glGenBuffers(1, &procVBO);
	glBindVertexArray(procVAO);
	glBindBuffer(GL_ARRAY_BUFFER, procVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatTotal, proceduralVerts, GL_STATIC_DRAW); // NEW ---------
		// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (vertexFloatTotal * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
		// Colour / UV
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (vertexFloatTotal * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
		// Normals | NOW TEXTURES
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (vertexFloatTotal * sizeof(float)), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	// ALPHA CHUNKS
	unsigned int alphVAO, alphVBO;
	glGenVertexArrays(1, &alphVAO);
	glGenBuffers(1, &alphVBO);
	glBindVertexArray(alphVAO);
	glBindBuffer(GL_ARRAY_BUFFER, alphVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * alphaVertexTotal, alphaVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (7 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (7 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	// Bezier Curves
	unsigned int curveVAO, curveVBO;
	glGenVertexArrays(1, &curveVAO);
	glGenBuffers(1, &curveVBO);
	glBindVertexArray(curveVAO);
	glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_curve_floats, curve_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int ctrlVAO, ctrlVBO;
	glGenVertexArrays(1, &ctrlVAO);
	glGenBuffers(1, &ctrlVBO);
	glBindVertexArray(ctrlVAO);
	glBindBuffer(GL_ARRAY_BUFFER, ctrlVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_ctrl_floats, ctrl_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	// SKYBOX
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND); // Blending 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Blend function

	glPointSize(8);

	// Transparency positions
	std::vector<glm::vec3> positions; // corresponds to the model translation per transparent obj
	positions.push_back(glm::vec3(-5060.f, 1800.f, -2350.f));
	positions.push_back(glm::vec3(-5060.f, 2800.f, -2350.f));
	positions.push_back(glm::vec3(-5060.f, 3800.f, -2350.f));
	
	//positions.push_back(glm::vec3(0.f, 1000.f, 0.f));
	//positions.push_back(glm::vec3(0.f, 2000.f, 0.f));
	//positions.push_back(glm::vec3(0.f, 3000.f, 0.f));


	// LOAD TEXTURES
	
	// Cubemap
	unsigned int skyboxTexture = loadCubemap(faces);
	
	// Objs
	unsigned int towerTexture = loadTexture("resources/textures/tower.jpeg");
	unsigned int tornadoTexture = loadTexture("resources/textures/tornado.png");
	unsigned int eagleTexture = loadTexture("resources/textures/eagle.png");

	// Chunks
	unsigned int rockTexture = loadTexture("resources/textures/rock.jpg");
	unsigned int grassTexture = loadTexture("resources/textures/grass.jpg");
	unsigned int waterTexture = loadTexture("resources/textures/water.jpg");
	unsigned int terrainTexture = loadTexture("resources/textures/terrain-atlas.jpg");

	while (!glfwWindowShouldClose(window))
	{
		static const GLfloat bgd[] = { .8f, .8f, .8f, 1.f };
		glClearBufferfv(GL_COLOR, 0, bgd);
		glClear(GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		// Constant view and projection
		glm::mat4 view = glm::mat4(1.f);
		view = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
		glm::mat4 projection = glm::mat4(1.f);
		projection = glm::perspective(glm::radians(fov), (float)WIDTH / (float)HEIGHT, .1f, 20000.f);


		// Procedural verts
		glUseProgram(textureProgram);

		// Copy light direction, light colour, camera position, light position (PHONG)
		//glUniform3f(glGetUniformLocation(phongProgram, "lightDirection"), lightDirection.x, lightDirection.y, lightDirection.z);
		//glUniform3f(glGetUniformLocation(phongProgram, "lightColour"), 1.f, 1.f, 1.f);
		//glUniform3f(glGetUniformLocation(phongProgram, "camPos"), Camera.Position.x, Camera.Position.y, Camera.Position.z);
		//glUniform3f(glGetUniformLocation(phongProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glm::mat4 model = glm::mat4(1.f);
		model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
		model = glm::translate(model, glm::vec3(.1f, -2.5f, 0.f));
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		
		glBindVertexArray(procVAO);
		glBindTexture(GL_TEXTURE_2D, rockTexture);
		glDrawArrays(GL_TRIANGLES, 0, vertexTotal);
		glBindVertexArray(0);

		// Tower
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));		
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6300.f, 80.f, -3500.f));
		model = glm::scale(model, glm::vec3(7500.f, 7500.f, 7500.f));
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		
		glBindVertexArray(towerVAO);
		glBindTexture(GL_TEXTURE_2D, towerTexture);
		// glActiveTexture(GL_TEXTURE0); // reflection, refraction ------------------------------------- (and switch program to appropriate program)
		// glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, towerVertsTotal);
		glBindVertexArray(0);




		// Tornado 1
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6000.f, 1100.f, -1500.f));
		model = glm::scale(model, glm::vec3(1250.f, 1250.f, 1250.f));
		model = glm::rotate(model, (float)glfwGetTime() / -0.5f, glm::vec3(0.0f, 1.0f, .0f));
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		
		glBindVertexArray(tornadoVAO);
		glBindTexture(GL_TEXTURE_2D, tornadoTexture);
		glDrawArrays(GL_TRIANGLES, 0, tornadoVertsTotal);
		glBindVertexArray(0);

		// Tornado 2
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6000.f, 1100.f, -5500.f));
		model = glm::scale(model, glm::vec3(1250.f, 1250.f, 1250.f));
		model = glm::rotate(model, (float)glfwGetTime() / 0.5f, glm::vec3(0.0f, 1.0f, .0f));
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		
		glBindVertexArray(tornadoVAO);
		glBindTexture(GL_TEXTURE_2D, tornadoTexture);
		glDrawArrays(GL_TRIANGLES, 0, tornadoVertsTotal);
		glBindVertexArray(0);

		// Eagle
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		
		// MAKE EAGLE FOLLOW CURVE -------------------------
		// CLOUDS INSTEAD ROTATE IN CIRCLE AROUND TOWER
		
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6300.f, 1000.f, -3500.f));

		//model = glm::translate(model, glm::vec3(-3000.f, 1000.f, 3000.f));
		//model = glm::rotate(model, (float)glfwGetTime() / 0.5f, glm::vec3(0.0f, 1.0f, .0f));
		//model = glm::translate(model, glm::vec3(6300.f, 1000.f, 3500.f));
		
		//model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.0f, 1.0f, .0f));
		model = glm::scale(model, glm::vec3(1000.f, 1000.f, 1000.f));
		glUniformMatrix4fv(glGetUniformLocation(textureProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		
		glBindVertexArray(eagleVAO);
		glBindTexture(GL_TEXTURE_2D, eagleTexture);
		glDrawArrays(GL_TRIANGLES, 0, eagleVertsTotal);
		glBindVertexArray(0);





		// Bind arrays: Bell
		glUseProgram(triangleProgram);

		glUniformMatrix4fv(glGetUniformLocation(triangleProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(triangleProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6300.f, 7050.f, -3500.f));
		model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
		model = glm::scale(model, glm::vec3(80.f, 80.f, 80.f));
		glUniformMatrix4fv(glGetUniformLocation(triangleProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		
		glBindVertexArray(bellVAO);
		glDrawArrays(GL_TRIANGLES, 0, bellVertsTotal);
		glBindVertexArray(0);
		
		// Clouds
			// also rotate, and swap between clouds and tornado






		// Bezier Curves
		glUseProgram(triangleProgram);

		model = glm::mat4(1.f);
		model = glm::scale(model, glm::vec3(80.f, 80.f, 80.f));
		glUniformMatrix4fv(glGetUniformLocation(triangleProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//view = glm::mat4(1.f);
		//view = glm::translate(view, -glm::vec3(0.f, 0.f, 12.f));
		glUniformMatrix4fv(glGetUniformLocation(triangleProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		//projection = glm::perspective(glm::radians(45.f), (float)800 / (float)600, .01f, 100.f);
		glUniformMatrix4fv(glGetUniformLocation(triangleProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		
		glBindVertexArray(curveVAO);
		glDrawArrays(GL_LINE_STRIP, 0, num_curve_verts);

		glBindVertexArray(ctrlVAO);
		glDrawArrays(GL_POINTS, 0, num_ctrl_verts);
		
		


		// Transparent verts
		glUseProgram(alphaProgram);
		
		std::map<float, glm::vec3> sorted_positions;
		for (int i = 0; i < positions.size(); i++)
		{
			glm::vec3 v = glm::vec3(Camera.Position.x, Camera.Position.y, Camera.Position.z) - positions[i];
			float l = glm::length(v);
			sorted_positions[l] = positions[i];
		}

		glBindVertexArray(alphVAO);
		for (auto it = sorted_positions.rbegin(); it != sorted_positions.rend(); ++it) // uses vector of sorted vectors (above) in reverse: "rbegin", "rend"
		{
			glUniformMatrix4fv(glGetUniformLocation(alphaProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(alphaProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);
			model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
			model = glm::scale(model, glm::vec3(3.3f, 3.3f, 3.3f));
			glUniformMatrix4fv(glGetUniformLocation(alphaProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, alphaVertexTotal);
		}



		// Skybox
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		
		glUseProgram(cubemapProgram);
		glUniformMatrix4fv(glGetUniformLocation(cubemapProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(cubemapProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-5000.f, 300.f, -3500.f));
		model = glm::scale(model, glm::vec3(10000.f, 10000.f, 10000.f));
		glUniformMatrix4fv(glGetUniformLocation(cubemapProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		
		glDepthFunc(GL_LESS); // set depth function back to default



		glfwSwapBuffers(window);

		glfwPollEvents();
		// processKeyboardRotate(window);
		processKeyboardWalkAround(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}