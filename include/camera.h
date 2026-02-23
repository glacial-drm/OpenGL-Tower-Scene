#pragma once

#include <stdio.h>
#include <glm/glm.hpp>

struct SCamera
{
	enum Camera_Movement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;

	glm::vec3 WorldUp;

	float Yaw;
	float Pitch;

	const float MovementSpeed = 5.5f;
	float MouseSensitivity = 1.f;



};


void InitCamera(SCamera& in)
{
	in.Front = glm::vec3(0.0f, 0.0f, -1.0f);
	in.Position = glm::vec3(0.0f, 0.0f, 0.0f);
	in.Up = glm::vec3(0.0f, 1.0f, 0.0f);
	in.WorldUp = in.Up;
	in.Right = glm::normalize(glm::cross(in.Front, in.WorldUp));

	in.Yaw = -90.f;
	in.Pitch = 0.f;
}

float cam_dist = 2.f;

void MoveAndOrientCamera(SCamera& in, glm::vec3 target,
	float distance, float xoffset, float yoffset) // Camera moves on an "imaginary sphere surrounding the model" - camera is reposition and reoriented at the same time
{
	// Params:
		// in - camera object
		// target - position in world space that is the focus of the camera
		// distance - distance from the camera to the focus traget
		// xoffset, yoffset - amount to move the camera around the imaginary sphere

	// The camera moves relative to the focused object
		// Hence changes to Yaw and Pitch are inverted
	in.Yaw -= xoffset;
	in.Pitch -= yoffset;

	in.Position = glm::vec3(glm::radians(), , );

}