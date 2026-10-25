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
#include "particle_generator.h"
#include "shadow.h"

#include "obj-load.h"
// #include "do_not_edit.h"

#define STB_IMAGE_IMPLEMENTATION // lines 4-13 in "stb_image.h"
#include "stb_image.h"

#define WIDTH 1024
#define HEIGHT 768
#define SH_MAP_WIDTH 2048
#define SH_MAP_HEIGHT 2048

//light direction & position variables here
glm::vec3 lightDirection = glm::vec3(0.f, -1.f, 0.f);
glm::vec3 lightPos = glm::vec3(0.f, 0.f, 0.f);

SCamera Camera;
ShadowStruct shadow;

std::map<string, GLuint> VAOs;
std::map<string, GLuint> VBOs;
std::map<string, int> floatTotals;
std::map<string, GLuint> shaderPrograms;
std::map<string, unsigned int> textures;

int num_curve_verts, num_ctrl_verts;
std::vector<point> curve;

ParticleGenerator* Particles;

bool firstMouse = true; // set to true initially as running program is first time mouse gets input
float lastX = 0.f, lastY = 0.f; // previous position of mouse on screen
float fov = 55.f;

bool weatherFlag = 0;


std::vector<glm::vec3> alphaChunkPositions; // corresponds to the model translation per transparent obj

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
vector<std::string> facesDark
{
	"resources/textures/skybox/rightDark.jpg",
	"resources/textures/skybox/leftDark.jpg",
	"resources/textures/skybox/topDark.jpg",
	"resources/textures/skybox/bottomDark.jpg",
	"resources/textures/skybox/frontDark.jpg",
	"resources/textures/skybox/backDark.jpg"
};

float sine(float f) {
	
	return glm::sin(glm::radians(f));
}
float csine(float f) {

	return glm::cos(glm::radians(f));
}

// Camera 
void processKeyboardRotate(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Set light direction & position to point from the current camera front / pos
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
	{
		lightDirection = Camera.Front;
		lightPos = Camera.Position;
	}
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	float cam_speed = 0.1f * deltaTime; //0.1f default

	float x_offset = 0.f;
	float y_offset = 0.f;
	bool cam_changed = false;
	// Rotate/Orient the camera in the specified direction (arrow keys)
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		x_offset = 0.f;
		y_offset = cam_speed;

		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		x_offset = 0.f;
		y_offset = -cam_speed;

		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		x_offset = -cam_speed;
		y_offset = 0.f;

		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		x_offset = cam_speed;
		y_offset = 0.f;

		cam_changed = true;
	}

	// Zoom in and out
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		cam_dist -= cam_speed / 10.f; // Declared in camera.h

		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		cam_dist += cam_speed / 10.f;

		cam_changed = true;
	}


	// Perform relevant changes
	if (cam_changed)
	{
		MoveAndOrientCamera(Camera, glm::vec3(-6300.f, 7000.f, -3500.f), cam_dist, x_offset, y_offset);
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

	// Set light direction & position to point from the current camera front / pos
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
	{
		lightDirection = Camera.Front;
		lightPos = Camera.Position;
	}

	// Rotate/Orient the camera in the specified direction (arrow keys)
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Camera.Position += cam_speed * Camera.Front;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		Camera.Position -= cam_speed * Camera.Front;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		Camera.Position -= glm::normalize(glm::cross(Camera.Front, Camera.Up)) * cam_speed;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		Camera.Position += glm::normalize(glm::cross(Camera.Front, Camera.Up)) * cam_speed;
	}
	
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
vector<float> parse_vertex(vertex v, bool isTex, bool alpha, bool isReflection, bool isTexPhong, bool randomAlpha, bool randomColour) {
	vector<float> params;

	glm::vec3 pos = v.pos;
	glm::vec3 col = v.col;
	glm::vec3 nor = v.nor;
	glm::vec2 tex = v.tex;
	float alp = v.alpha;

	params = { pos.x, pos.y, pos.z, col.r, col.g, col.b, nor.x, nor.y, nor.z };
	
	if (randomColour) {
		int offset = static_cast <float>(rand()) / static_cast <float>(RAND_MAX);
		col.x -= offset;
		col.y -= offset;
		col.z -= offset;
	}

	if (isTex) {
		params = { pos.x, pos.y, pos.z, col.r, col.g, col.b, tex.x, tex.y};
		// params = { pos.x, pos.y, pos.z, nor.x, nor.y, nor.z, tex.x, tex.y }; // passing normals instead of colour
	}

	if (isReflection) {
		params = { pos.x, pos.y, pos.z, nor.x, nor.y, nor.z };
	}

	if (isTexPhong) {
		params = { pos.x, pos.y, pos.z, tex.x, tex.y, nor.x, nor.y, nor.z };
	}

	if (alpha) {
		if (randomAlpha) {
			alp = static_cast <float>(rand()) / static_cast <float>(RAND_MAX);
		}

		params.push_back(alp);
	}
	
	return params;
}

float* load_model(const std::string MODEL_PATH, int &floatTotalOutput, bool isTex, bool alpha, bool isReflection, bool isTexPhong, bool randomAlpha, bool randomColour) {
	
	// bool isTex: either texture or colour
	// bool alpha: alpha values

	std::vector<float> verts_vect;

	std::vector<triangle> tris;
	obj_parse(MODEL_PATH.c_str(), &tris);

	for (triangle t : tris) { // index over triangles 

		std::vector<float> vert = parse_vertex(t.v1, isTex, alpha, isReflection, isTexPhong, randomAlpha, randomColour); // parse info we need (pos, col, nor)
		verts_vect.insert(verts_vect.end(), vert.begin(), vert.end());

		vert = parse_vertex(t.v2, isTex, alpha, isReflection, isTexPhong, randomAlpha, randomColour);
		verts_vect.insert(verts_vect.end(), vert.begin(), vert.end());

		vert = parse_vertex(t.v3, isTex, alpha, isReflection, isTexPhong, randomAlpha, randomColour);
		verts_vect.insert(verts_vect.end(), vert.begin(), vert.end());

	}

	// get tris and convert to format in vertices[]
	floatTotalOutput = verts_vect.size();
	float* verts = new float[floatTotalOutput];
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


bool weatherCondition() {

	if (weatherFlag /*&& glm::distance(Camera.Position, glm::vec3(-6200.f, 7600.f, -3400.f)) < 3000.f*/) {

		return true;
	}

	return false;
}


void drawWithShadow() {
	// Constant view and projection
	glm::mat4 view = glm::mat4(1.f);
	view = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
	glm::mat4 projection = glm::mat4(1.f);
	projection = glm::perspective(glm::radians(fov), (float)WIDTH / (float)HEIGHT, .1f, 20000.f);


	float near_plane = 0.0f, far_plane = 20000.f; // taken from projection matrix in draw()
	// projection = glm::ortho(-10.f, -6000.0f, -10.f, 8000.f, near_plane, far_plane);
	// view = glm::lookAt(lightPos, lightPos + lightDirection, glm::vec3(0.0f, 1.f, 0.0f)); // lookAt() constructs a view matrix from light position, light direction and up vectors

	// Procedural verts
	glUseProgram(shaderPrograms["shadow"]);


	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glm::mat4 model = glm::mat4(1.f);
	model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
	model = glm::translate(model, glm::vec3(.1f, -2.5f, 0.f));
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "model"), 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAOs["preset"]);

	// Interactive obj / weather check
	cout << Camera.Position.x << " " << Camera.Position.y << " " << Camera.Position.z << endl;
	// cout << glm::distance(Camera.Position, glm::vec3(-6200.f, 7600.f, -3400.f));
	glBindTexture(GL_TEXTURE_2D, textures["terrain"]);
	glDrawArrays(GL_TRIANGLES, 0, floatTotals["vertexTotal"]);
	glBindVertexArray(0);

	// Tower
	glUseProgram(shaderPrograms["shadow"]);
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-6300.f, 80.f, -3500.f));
	model = glm::scale(model, glm::vec3(7500.f, 7500.f, 7500.f));
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "model"), 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAOs["tower"]);

	if (weatherCondition()) {
		glBindTexture(GL_TEXTURE_2D, textures["tower-dark"]);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, textures["tower"]);
	}
	glDrawArrays(GL_TRIANGLES, 0, floatTotals["tower"]);
	glBindVertexArray(0);



	if (weatherCondition()) {
		// Tornado 1
		glUseProgram(shaderPrograms["shadow"]);
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);

		//model = glm::translate(model, glm::vec3(-6000.f, 1400.f, -5500.f)); // original tornado
		//model = glm::scale(model, glm::vec3(1250.f, 3000.f, 1250.f));

		model = glm::translate(model, glm::vec3(-6000.f, -520.f, -1500.f));
		model = glm::scale(model, glm::vec3(1.2f, 7.f, 1.2f));
		model = glm::rotate(model, (float)glfwGetTime() / 0.1f, glm::vec3(0.0f, 1.0f, .0f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOs["tornado"]);
		// glBindTexture(GL_TEXTURE_2D, textures["tornado"]);
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["tornado"]);
		glBindVertexArray(0);

		// Tornado 2
		glUseProgram(shaderPrograms["shadow"]);
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6000.f, -520.f, -5500.f));
		model = glm::scale(model, glm::vec3(1.2f, 7.f, 1.2f));
		model = glm::rotate(model, (float)glfwGetTime() / -0.1f, glm::vec3(0.0f, 1.0f, .0f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOs["tornado"]);
		// glBindTexture(GL_TEXTURE_2D, textures["tornado"]);
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["tornado"]);
		glBindVertexArray(0);


		// Bezier Curve 1
		glUseProgram(shaderPrograms["shadow"]);

		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6000.f, 0.f, -1300.f));
		model = glm::rotate(model, (float)glfwGetTime() / -0.1f, glm::vec3(0.0f, 1.0f, .0f));
		model = glm::scale(model, glm::vec3(90.f, 90.f, 90.f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOs["curve"]);
		glDrawArrays(GL_LINE_STRIP, 0, num_curve_verts);

		//glBindVertexArray(VAOs["control"]);
		//glDrawArrays(GL_POINTS, 0, num_ctrl_verts);

		// Bezier Curve 2
		glUseProgram(shaderPrograms["shadow"]);

		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6000.f, 0.f, -5600.f));
		model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.0f, 1.0f, .0f));
		model = glm::rotate(model, (float)glfwGetTime() / 0.1f, glm::vec3(0.0f, 1.0f, .0f));
		model = glm::scale(model, glm::vec3(90.f, 90.f, 90.f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOs["curve"]);
		glDrawArrays(GL_LINE_STRIP, 0, num_curve_verts);

		//glBindVertexArray(VAOs["control"]);
		//glDrawArrays(GL_POINTS, 0, num_ctrl_verts);

	}
	else {

		// CLOUDS INSTEAD ROTATE IN CIRCLE AROUND TOWER
		glUseProgram(shaderPrograms["shadow"]);
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6300.f, 1000.f, -3700.f));
		model = glm::rotate(model, (float)glfwGetTime() / 0.5f, glm::vec3(0.0f, 1.0f, .0f));
		model = glm::scale(model, glm::vec3(30.f, 30.f, 30.f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		// cloud 2
		glBindVertexArray(VAOs["cloudRing"]);
		// glBindTexture(GL_TEXTURE_2D, textures["cloudRing"]);
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["cloudRing"]);
		glBindVertexArray(0);

		glUseProgram(shaderPrograms["shadow"]);
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6300.f, 3000.f, -3700.f));
		model = glm::rotate(model, (float)glfwGetTime() / -1.f, glm::vec3(0.0f, 1.0f, .0f));
		model = glm::scale(model, glm::vec3(32.f, 32.f, 32.f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOs["cloudRing"]);
		// glBindTexture(GL_TEXTURE_2D, textures["cloudRing"]);
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["cloudRing"]);
		glBindVertexArray(0);

		// cloud 3
		glBindVertexArray(VAOs["cloudRing"]);
		// glBindTexture(GL_TEXTURE_2D, textures["cloudRing"]);
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["cloudRing"]);
		glBindVertexArray(0);

		glUseProgram(shaderPrograms["shadow"]);
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6300.f, 5000.f, -3700.f));
		model = glm::rotate(model, (float)glfwGetTime() / 2.f, glm::vec3(0.0f, 1.0f, .0f));
		model = glm::scale(model, glm::vec3(35.f, 35.f, 35.f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOs["cloudRing"]);
		// glBindTexture(GL_TEXTURE_2D, textures["cloudRing"]);
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["cloudRing"]);
		glBindVertexArray(0);

	}

	// Bind arrays: Bell
	GLuint program = shaderPrograms["shadow"];
	if (weatherCondition()) {
		program = shaderPrograms["shadow"];
	}

	glUseProgram(program);

	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-6300.f, 7080.f, -3500.f));
	model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
	model = glm::scale(model, glm::vec3(80.f, 80.f, 80.f));
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAOs["bell"]);
	glActiveTexture(GL_TEXTURE0); // reflection, refraction ------------------------------------- (and switch program to appropriate program)
	if (weatherCondition()) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, textures["skybox-dark"]);
	}
	else {
		glBindTexture(GL_TEXTURE_CUBE_MAP, textures["skybox"]);
	}
	glDrawArrays(GL_TRIANGLES, 0, floatTotals["bell"]);
	glBindVertexArray(0);


	Particles->Draw(view, projection, model);
	Particles->Update(0.1f, glm::vec3(-90.f, 70.f, -45.f), 2, glm::vec3(2.0f)); // ParticleGenerator::Update(float dt, glm::vec2 position, unsigned int newParticles, glm::vec2 offset, glm::vec2 velocity)

	// Transparent verts
	glUseProgram(shaderPrograms["shadow"]);

	std::map<float, glm::vec3> sorted_positions;
	for (int i = 0; i < alphaChunkPositions.size(); i++)
	{
		glm::vec3 v = glm::vec3(Camera.Position.x, Camera.Position.y, Camera.Position.z) - alphaChunkPositions[i];
		float l = glm::length(v);
		sorted_positions[l] = alphaChunkPositions[i];
	}

	glBindVertexArray(VAOs["alpha"]);
	for (auto it = sorted_positions.rbegin(); it != sorted_positions.rend(); ++it) // uses vector of sorted vectors (above) in reverse: "rbegin", "rend"
	{
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.0f);
		model = glm::translate(model, it->second);
		model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
		model = glm::scale(model, glm::vec3(3.3f, 3.3f, 3.3f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["alpha"]);
	}

	// Skybox
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

	glUseProgram(shaderPrograms["shadow"]);
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-5000.f, 300.f, -3500.f));
	model = glm::scale(model, glm::vec3(10000.f, 10000.f, 10000.f));
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["shadow"], "model"), 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAOs["skybox"]);
	glActiveTexture(GL_TEXTURE0);
	if (weatherCondition()) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, textures["skybox-dark"]);
	}
	else {
		glBindTexture(GL_TEXTURE_CUBE_MAP, textures["skybox"]);
	}
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS); // set depth function back to default
}









void draw(glm::mat4 view) {
	// Constant view and projection
	glm::mat4 projection = glm::mat4(1.f);
	projection = glm::perspective(glm::radians(fov), (float)WIDTH / (float)HEIGHT, .1f, 20000.f);
	

	float near_plane = 0.0f, far_plane = 20000.f; // taken from projection matrix in draw()
	glm::mat4 lightProjection = glm::ortho(-10.f, -6000.0f, -10.f, 8000.f, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightDirection, glm::vec3(0.0f, 1.f, 0.0f));
	// lightView = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
	glm::mat4 projectedLightSpaceMatrix = lightProjection * lightView; // create projected light space matrix by multiplying the light projection and light view matrices

	// Phong lighting parameters
	glUseProgram(shaderPrograms["phong-texture-spot"]);
	glBindTexture(GL_TEXTURE_2D, shadow.Texture);
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-texture-spot"], "projectedLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(projectedLightSpaceMatrix));
	glUniform3f(glGetUniformLocation(shaderPrograms["phong-texture-spot"], "lightDirection"), lightDirection.x, lightDirection.y, lightDirection.z);
	glUniform3f(glGetUniformLocation(shaderPrograms["phong-texture-spot"], "lightColour"), 1.f, 1.f, 1.f);
	glUniform3f(glGetUniformLocation(shaderPrograms["phong-texture-spot"], "camPos"), Camera.Position.x, Camera.Position.y, Camera.Position.z);
	glUniform3f(glGetUniformLocation(shaderPrograms["phong-texture-spot"], "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	glUseProgram(shaderPrograms["phong-directional"]);
	//glBindTexture(GL_TEXTURE_2D, shadow.Texture);
	//glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "projectedLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(projectedLightSpaceMatrix));
	glUniform3f(glGetUniformLocation(shaderPrograms["phong-directional"], "lightDirection"), lightDirection.x, lightDirection.y, lightDirection.z);
	glUniform3f(glGetUniformLocation(shaderPrograms["phong-directional"], "lightColour"), 1.f, 1.f, 1.f);
	glUniform3f(glGetUniformLocation(shaderPrograms["phong-directional"], "camPos"), Camera.Position.x, Camera.Position.y, Camera.Position.z);
	glUniform3f(glGetUniformLocation(shaderPrograms["phong-directional"], "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	glUseProgram(shaderPrograms["phong-positional"]);
	glUniform3f(glGetUniformLocation(shaderPrograms["phong-positional"], "lightDirection"), lightDirection.x, lightDirection.y, lightDirection.z);
	glUniform3f(glGetUniformLocation(shaderPrograms["phong-positional"], "lightColour"), 1.f, 1.f, 1.f);
	glUniform3f(glGetUniformLocation(shaderPrograms["phong-positional"], "camPos"), Camera.Position.x, Camera.Position.y, Camera.Position.z);
	glUniform3f(glGetUniformLocation(shaderPrograms["phong-positional"], "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	// Procedural verts
	glUseProgram(shaderPrograms["texture"]);

	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["texture"], "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["texture"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glm::mat4 model = glm::mat4(1.f);
	model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
	model = glm::translate(model, glm::vec3(.1f, -100.f, 0.f));
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["texture"], "model"), 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAOs["preset"]);

	// Interactive obj / weather check
	// cout << Camera.Position.x << " " << Camera.Position.y << " " << Camera.Position.z << endl;
	// cout << glm::distance(Camera.Position, glm::vec3(-6200.f, 7600.f, -3400.f));
	if (weatherCondition()) {
		glBindTexture(GL_TEXTURE_2D, textures["waterDark"]);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, textures["water"]);
	}
	

	glDrawArrays(GL_TRIANGLES, 0, floatTotals["vertexTotal"]);
	glBindVertexArray(0);

	// Tower
	glUseProgram(shaderPrograms["phong-texture-spot"]);
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-texture-spot"], "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-texture-spot"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-6300.f, 80.f, -3500.f));
	model = glm::scale(model, glm::vec3(7500.f, 7500.f, 7500.f));
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-texture-spot"], "model"), 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAOs["tower"]);
		
	if (weatherCondition()) {
		glBindTexture(GL_TEXTURE_2D, textures["tower-dark"]);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, textures["tower"]);
	}
	glDrawArrays(GL_TRIANGLES, 0, floatTotals["tower"]);
	glBindVertexArray(0);



	if(weatherCondition()){
		// Tornado 1
		glUseProgram(shaderPrograms["phong-positional"]);
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-positional"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-positional"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);

		//model = glm::translate(model, glm::vec3(-6000.f, 1400.f, -5500.f)); // original tornado
		//model = glm::scale(model, glm::vec3(1250.f, 3000.f, 1250.f));

		model = glm::translate(model, glm::vec3(-6000.f, -520.f, -1500.f));
		model = glm::scale(model, glm::vec3(1.2f, 7.f, 1.2f));
		model = glm::rotate(model, (float)glfwGetTime() / 0.1f, glm::vec3(0.0f, 1.0f, .0f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-positional"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOs["tornado"]);
		// glBindTexture(GL_TEXTURE_2D, textures["tornado"]);
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["tornado"]);
		glBindVertexArray(0);

		// Tornado 2
		glUseProgram(shaderPrograms["phong-directional"]);
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6000.f, -520.f, -5500.f));
		model = glm::scale(model, glm::vec3(1.2f, 7.f, 1.2f));
		model = glm::rotate(model, (float)glfwGetTime() / -0.1f, glm::vec3(0.0f, 1.0f, .0f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOs["tornado"]);
		// glBindTexture(GL_TEXTURE_2D, textures["tornado"]);
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["tornado"]);
		glBindVertexArray(0);


		// Bezier Curve 1
		glUseProgram(shaderPrograms["triangle"]);

		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["triangle"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["triangle"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6000.f, 0.f, -1300.f));
		model = glm::rotate(model, (float)glfwGetTime() / -0.1f, glm::vec3(0.0f, 1.0f, .0f));
		model = glm::scale(model, glm::vec3(90.f, 90.f, 90.f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["triangle"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOs["curve"]);
		glDrawArrays(GL_LINE_STRIP, 0, num_curve_verts);

		//glBindVertexArray(VAOs["control"]);
		//glDrawArrays(GL_POINTS, 0, num_ctrl_verts);

		// Bezier Curve 2
		glUseProgram(shaderPrograms["triangle"]);

		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["triangle"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["triangle"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6000.f, 0.f, -5600.f));
		model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.0f, 1.0f, .0f));
		model = glm::rotate(model, (float)glfwGetTime() / 0.1f, glm::vec3(0.0f, 1.0f, .0f));
		model = glm::scale(model, glm::vec3(90.f, 90.f, 90.f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["triangle"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOs["curve"]);
		glDrawArrays(GL_LINE_STRIP, 0, num_curve_verts);

		//glBindVertexArray(VAOs["control"]);
		//glDrawArrays(GL_POINTS, 0, num_ctrl_verts);

	}
	else{

		// CLOUDS INSTEAD ROTATE IN CIRCLE AROUND TOWER
		glUseProgram(shaderPrograms["phong-directional"]);
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6300.f, 1000.f, -3700.f));
		model = glm::rotate(model, (float)glfwGetTime() / 0.5f, glm::vec3(0.0f, 1.0f, .0f));
		model = glm::scale(model, glm::vec3(30.f, 30.f, 30.f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		// cloud 2
		glBindVertexArray(VAOs["cloudRing"]);
		// glBindTexture(GL_TEXTURE_2D, textures["cloudRing"]);
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["cloudRing"]);
		glBindVertexArray(0);

		glUseProgram(shaderPrograms["phong-directional"]);
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6300.f, 3000.f, -3700.f));
		model = glm::rotate(model, (float)glfwGetTime() / -1.f, glm::vec3(0.0f, 1.0f, .0f));
		model = glm::scale(model, glm::vec3(32.f, 32.f, 32.f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOs["cloudRing"]);
		// glBindTexture(GL_TEXTURE_2D, textures["cloudRing"]);
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["cloudRing"]);
		glBindVertexArray(0);

		// cloud 3
		glBindVertexArray(VAOs["cloudRing"]);
		// glBindTexture(GL_TEXTURE_2D, textures["cloudRing"]);
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["cloudRing"]);
		glBindVertexArray(0);

		glUseProgram(shaderPrograms["phong-directional"]);
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(-6300.f, 5000.f, -3700.f));
		model = glm::rotate(model, (float)glfwGetTime() / 2.f, glm::vec3(0.0f, 1.0f, .0f));
		model = glm::scale(model, glm::vec3(35.f, 35.f, 35.f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["phong-directional"], "model"), 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOs["cloudRing"]);
		// glBindTexture(GL_TEXTURE_2D, textures["cloudRing"]);
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["cloudRing"]);
		glBindVertexArray(0);
	
	}

	// Bind arrays: Bell
	GLuint program = shaderPrograms["reflection"];
	if (weatherCondition()) {
		program = shaderPrograms["refraction"];
	}

	glUseProgram(program);

	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-6300.f, 7080.f, -3500.f));
	model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
	model = glm::scale(model, glm::vec3(80.f, 80.f, 80.f));
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAOs["bell"]);
	glActiveTexture(GL_TEXTURE0); // reflection, refraction
	if (weatherCondition()) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, textures["skybox-dark"]);
	}
	else {
		glBindTexture(GL_TEXTURE_CUBE_MAP, textures["skybox"]);
	}
	glDrawArrays(GL_TRIANGLES, 0, floatTotals["bell"]);
	glBindVertexArray(0);

	// particles 
	model = glm::mat4(1.f);
	model = glm::scale(model, glm::vec3(80.f, 80.f, 80.f));
	Particles->Draw(view, projection, model);


	// Transparent verts
	glUseProgram(shaderPrograms["alpha"]);

	std::map<float, glm::vec3> sorted_positions;
	for (int i = 0; i < alphaChunkPositions.size(); i++)
	{
		glm::vec3 v = glm::vec3(Camera.Position.x, Camera.Position.y, Camera.Position.z) - alphaChunkPositions[i];
		float l = glm::length(v);
		sorted_positions[l] = alphaChunkPositions[i];
	}

	glBindVertexArray(VAOs["alpha"]);
	for (auto it = sorted_positions.rbegin(); it != sorted_positions.rend(); ++it) // uses vector of sorted vectors (above) in reverse: "rbegin", "rend"
	{
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["alpha"], "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["alpha"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1.0f);
		model = glm::translate(model, it->second);
		model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
		model = glm::scale(model, glm::vec3(3.3f, 3.3f, 3.3f));
		glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["alpha"], "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, floatTotals["alpha"]);
	}

	// Skybox
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

	glUseProgram(shaderPrograms["cubemap"]);
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["cubemap"], "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["cubemap"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-5000.f, 300.f, -3500.f));
	model = glm::scale(model, glm::vec3(10000.f, 10000.f, 10000.f));
	glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["cubemap"], "model"), 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAOs["skybox"]);
	glActiveTexture(GL_TEXTURE0);
	if (weatherCondition()) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, textures["skybox-dark"]);
	}
	else {
		glBindTexture(GL_TEXTURE_CUBE_MAP, textures["skybox"]);
	}
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS); // set depth function back to default
}

// Shadow
void generateDepthMap(unsigned int shadowShaderProgram, ShadowStruct shadow, glm::mat4 projectedLightSpaceMatrix)
{

	glViewport(0, 0, SH_MAP_WIDTH, SH_MAP_HEIGHT);


	glBindFramebuffer(GL_FRAMEBUFFER, shadow.FBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Render scene using model matrices
	glUseProgram(shadowShaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "projectedLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(projectedLightSpaceMatrix));

	drawWithShadow(); // draw and write z-values to framebuffer in shadow structure

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

	// set up the view and projection matrices
	glm::mat4 view = glm::mat4(1.f);
	view = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
	glUniformMatrix4fv(glGetUniformLocation(renderShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	//glm::mat4 projection = glm::mat4(1.f);
	//projection = glm::perspective(glm::radians(45.f), (float)WIDTH / (float)HEIGHT, .01f, 100.f);
	//glUniformMatrix4fv(glGetUniformLocation(renderShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// draw for real 
	draw(view);
}



int main(int argc, char** argv)
{
	// PRESET CHUNKS

	glm::vec3 origin = glm::vec3(-5000.f, 0.f, -6000.f); // glm::vec3(0.f, 0.f, 0.f); 
	int originOffset = (dim - 1) * stride; // -1 as dim is array size 32 compared to 0-31 range

	int chunksX = 12, chunksZ = 10; // corresponds to maxHeights arr
	int chunkVertTotal = dim * dim * 6; // *6 = *3 (triangle) * 2 (2 triangles per square)
	floatTotals["vertexTotal"] = chunkVertTotal * chunksX * chunksZ; // procedural | testVertices // 

	int chunkVertexFloatTotal = 8; // 9 for colour (3 components) // 8 for texture (2 components)
	int chunkFloatSize = chunkVertTotal * chunkVertexFloatTotal; // *9 as 9 components per vert (3 pos, 3 col, 3 nor)
	int floatTotal = chunkFloatSize * chunksX * chunksZ;

	std::vector<float> testVertsVect;
	float* proceduralVerts = new float[floatTotal];

	float* loopVerts = new float[chunkFloatSize]; // single chunk iteration

	int gridOffset;
	chunk** loopChunks = (chunk**)malloc((chunksX * chunksZ) * sizeof(chunk*)); // grid of chunks
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
				loopChunk = createChunk(origin, maxHeights[(chunksX - 1) - row][col], true, 250); // chunksX - row to account for 2d array indexing (0,0 top left) vs chunk indexing (0,0 bottom left)
			}
			else {
				loopChunk = createChunk(origin, maxHeights[(chunksX - 1) - row][col], true, 250);
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
			loopVerts = chunkVertices(loopChunk, chunkVertexFloatTotal);
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
	floatTotals["alpha"] = dim * dim * 6 * 7; // single chunk procedural
	glm::vec3 towerOrigin = glm::vec3(0.f, 0.f, 0.f);// glm::vec3(-6300.f, 80.f + 2000.f , -3500.f);
	chunk* alphaChunk = createChunk(towerOrigin, 2, true, 98); 
	float* alphaVerts = chunkVertices(alphaChunk, 7);


	// load models 
	float* towerVerts = load_model("resources/objs/tower.obj", floatTotals["tower"], false, false, false, true, false, false);
	float* tornadoVerts = load_model("resources/objs/tornado.obj", floatTotals["tornado"], false, false, false, false, false, true);
	float* bellVerts = load_model("resources/objs/bell.obj", floatTotals["bell"], false, false, true, false, false, false);
	float* cloudRingVerts = load_model("resources/objs/cloudRing.obj", floatTotals["cloudRing"], false, false, false, false, false, true);


	// BEZIER CURVE 
	
	std::vector<float> angles;
	float rotations = 1.5;
	int angleTotal = 20;
	for (int i = 0; i <= angleTotal; i++) {
		angles.push_back((float)i * ((360.f * rotations)/angleTotal));
	}
	
	float radius = 10;
	float heightOffset = 3.f;
	std::vector<point> ctrl_points;
	for (int i = 0; i <= angleTotal; i++) {
		ctrl_points.push_back(point(sine(angles[i]) * radius, (float)(i+1) * heightOffset, csine(angles[i]) * radius));
	}

	int num_evals = 30;
	curve = EvaluateBezierCurve(ctrl_points, num_evals);

	
	float* curve_vertices = MakeFloatsFromVector(curve, num_curve_verts, floatTotals["curve"], 1.f, 1.f, 1.f);
	float* ctrl_vertices = MakeFloatsFromVector(ctrl_points, num_ctrl_verts, floatTotals["control"], 1.f, 1.f, 0.f);



	glfwInit();

	// glfwWindowHint(GLFW_SAMPLES, 8); // 8x multisampling anti aliasing
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Lighting", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, SizeCallback);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	gl3wInit();


	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugCallback, 0);

	shaderPrograms["cubemap"] = CompileShader("skybox.vert", "skybox.frag");
	shaderPrograms["alpha"] = CompileShader("alpha.vert", "alpha.frag");
	shaderPrograms["reflection"] = CompileShader("reflection.vert", "reflection.frag");
	shaderPrograms["refraction"] = CompileShader("reflection.vert", "refraction.frag");
	shaderPrograms["particle"] = CompileShader("particle.vert", "particle.frag");
	shaderPrograms["phong-texture-spot"] = CompileShader("phongTextureSpot.vert", "phongTextureSpot.frag");

	shaderPrograms["phong-shadow"] = CompileShader("phongShadow.vert", "phongShadow.frag");
	shaderPrograms["phong-directional"] = CompileShader("phongShadow.vert", "phongDirectional.frag");
	shaderPrograms["phong-positional"] = CompileShader("phong.vert", "phongPositional.frag");
	shaderPrograms["shadow"] = CompileShader("shadow.vert", "shadow.frag");

	shaderPrograms["triangle"] = CompileShader("triangle.vert", "triangle.frag");
	shaderPrograms["texture"] = CompileShader("texture.vert", "texture.frag");

	

	
	InitCamera(Camera);


	// PRESET CHUNKS
		// separate as uses "chunkVertexFloatTotal" and "floatTotal"
			// could move into preset func
	glGenVertexArrays(1, &VAOs["preset"]);
	glGenBuffers(1, &VBOs["preset"]);
	glBindVertexArray(VAOs["preset"]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs["preset"]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* floatTotal, proceduralVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (chunkVertexFloatTotal * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (chunkVertexFloatTotal * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (chunkVertexFloatTotal * sizeof(float)), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	// PER OBJ BASIS
		// TOWER -> PILLARS -> could pre render
	glGenVertexArrays(1, &VAOs["tower"]);
	glGenBuffers(1, &VBOs["tower"]);
	glBindVertexArray(VAOs["tower"]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs["tower"]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatTotals["tower"], towerVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (8 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (8 * sizeof(float)), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	
	glGenVertexArrays(1, &VAOs["tornado"]);
	glGenBuffers(1, &VBOs["tornado"]);
	glBindVertexArray(VAOs["tornado"]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs["tornado"]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatTotals["tornado"], tornadoVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	//glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, (10 * sizeof(float)), (void*)(9 * sizeof(float)));
	//glEnableVertexAttribArray(3);

	glGenVertexArrays(1, &VAOs["cloudRing"]);
	glGenBuffers(1, &VBOs["cloudRing"]);
	glBindVertexArray(VAOs["cloudRing"]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs["cloudRing"]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* floatTotals["cloudRing"], cloudRingVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glGenVertexArrays(1, &VAOs["bell"]);
	glGenBuffers(1, &VBOs["bell"]);
	glBindVertexArray(VAOs["bell"]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs["bell"]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatTotals["bell"], bellVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)(6 * sizeof(float)));
	//glEnableVertexAttribArray(2);

	// ALPHA CHUNKS
	glGenVertexArrays(1, &VAOs["alpha"]);
	glGenBuffers(1, &VBOs["alpha"]);
	glBindVertexArray(VAOs["alpha"]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs["alpha"]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatTotals["alpha"], alphaVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (7 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (7 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Bezier Curves
	glGenVertexArrays(1, &VAOs["curve"]);
	glGenBuffers(1, &VBOs["curve"]);
	glBindVertexArray(VAOs["curve"]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs["curve"]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatTotals["curve"], curve_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glGenVertexArrays(1, &VAOs["control"]);
	glGenBuffers(1, &VBOs["control"]);
	glBindVertexArray(VAOs["control"]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs["control"]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatTotals["control"], ctrl_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	// SKYBOX
	glGenVertexArrays(1, &VAOs["skybox"]);
	glGenBuffers(1, &VBOs["skybox"]);
	glBindVertexArray(VAOs["skybox"]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs["skybox"]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	
	
	// configure MSAA framebuffer
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// create a multisampled color attachment texture
	unsigned int textureColorBufferMultiSampled;
	glGenTextures(1, &textureColorBufferMultiSampled);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, WIDTH, HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);

	// create a (also multisampled) renderbuffer object for depth and stencil attachments
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// configure second post-processing framebuffer
	unsigned int intermediateFBO;
	glGenFramebuffers(1, &intermediateFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

	// create a color attachment texture
	unsigned int screenTexture;
	glGenTextures(1, &screenTexture);
	glBindTexture(GL_TEXTURE_2D, screenTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	// we only need a color buffer

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	glEnable(GL_DEPTH_TEST); // Depth test

	glEnable(GL_MULTISAMPLE); // Multisampling for anti-aliasing, redundant

	glEnable(GL_BLEND); // Blending 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Blend function

	glPointSize(8);

	// Transparency positions
	alphaChunkPositions.push_back(glm::vec3(-5060.f, 1800.f, -2350.f));
	alphaChunkPositions.push_back(glm::vec3(-5060.f, 2800.f, -2350.f));
	alphaChunkPositions.push_back(glm::vec3(-5060.f, 3800.f, -2350.f));


	// LOAD TEXTURES

	// Cubemap
	textures["skybox"] = loadCubemap(faces);
	textures["skybox-dark"] = loadCubemap(facesDark);

	// Objs
	textures["tower"] = loadTexture("resources/textures/tower.jpeg");
	textures["tower-dark"] = loadTexture("resources/textures/towerDark.jpg");

	// Chunks 
	textures["water"] = loadTexture("resources/textures/water.jpg");
	textures["waterDark"] = loadTexture("resources/textures/waterDark.jpg");

	// Particle
	textures["sparkle"] = loadTexture("resources/textures/sparkles.png");
	
	// Particles and generation
	Particles = new ParticleGenerator(
		shaderPrograms["particle"],
		textures["sparkle"],
		5
	);


	// Shadows
	shadow = setup_shadowmap(SH_MAP_WIDTH, SH_MAP_HEIGHT);

	while (!glfwWindowShouldClose(window))
	{
		static const GLfloat bgd[] = { .8f, .8f, .8f, 1.f };
		glClearBufferfv(GL_COLOR, 0, bgd);
		glClear(GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// shadow setup
		float near_plane = 0.0f, far_plane = 20000.f; // taken from projection matrix in draw()
		glm::mat4 lightProjection = glm::ortho(-10.f, -6000.0f, -10.f, 8000.f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightDirection, glm::vec3(0.0f, 1.f, 0.0f));
		glm::mat4 projectedLightSpaceMatrix = lightProjection * lightView; // create projected light space matrix by multiplying the light projection and light view matrices

		generateDepthMap(shaderPrograms["shadow"], shadow, projectedLightSpaceMatrix);
		//saveShadowMapToBitmap(shadow.Texture, SH_MAP_WIDTH, SH_MAP_HEIGHT);
		// renderWithShadow(shaderPrograms["phong-shadow"], shadow, projectedLightSpaceMatrix);
		
		// Cameras
		glm::mat4 view = glm::mat4(1.0f);
		if (weatherCondition()) { // for above cameras rip
			float radius = 5000.0f;
			float camX = static_cast<float>(sin(glfwGetTime()) * radius);
			float camZ = static_cast<float>(cos(glfwGetTime()) * radius);
			view = glm::lookAt(glm::vec3(camX, 7000.f, camZ), glm::vec3(-6300.f, 7000.f, -3500.f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else {
			view = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
		}
		
		glViewport(0, 0, WIDTH, HEIGHT);
		draw(view);

		glfwSwapBuffers(window);

		glfwPollEvents();

		if(! weatherCondition()){
			processKeyboardWalkAround(window);
		}


		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
		{
			weatherFlag = false;
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			weatherFlag = true;
		}
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}