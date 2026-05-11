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

	const float MovementSpeed = .5f;
	float MouseSensitivity = 1.f;

	glm::vec3 Target;

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

	in.Target = glm::vec3(0.f, 0.f, 0.f);
}

float cam_dist = 5.0f;

void cameraRotateAroundObject(SCamera& in, glm::vec3 target,
	float distance, float xoffset, float yoffset) // Camera moves on an "imaginary sphere surrounding the model" - camera is reposition and reoriented at the same time
{
	// Params:
		// in - camera object
		// target - position in world space that is the focus of the camera
			// NOW SEPARATE FROM THE STRUCT'S TARGET (the one there may not be permanent)
			// make the obj's position in space the target
		// distance - distance from the camera to the focus traget
		// xoffset, yoffset - amount to move the camera around the imaginary sphere

	// The camera moves relative to the focused object
		// Hence changes to Yaw and Pitch are inverted
	in.Yaw -= xoffset * in.MovementSpeed;
	in.Pitch -= yoffset * in.MovementSpeed;

	// Up and Down directions have strange orientataion at poles of the unit sphere defining camera path
		// Fix is to limit the camera pitch to between -89 and 89
	if (in.Pitch > 89.0f)	in.Pitch = 89.0f;
	if (in.Pitch < -89.0f)	in.Pitch = -89.0f;

	// Update Camera Position using newly calced Yaw and Pitch
		// Rotate around each of the axes
	float px = cos(glm::radians(in.Yaw)) * cos(glm::radians(in.Pitch));
	float py = sin(glm::radians(in.Pitch));
	float pz = sin(glm::radians(in.Yaw)) * cos(glm::radians(in.Pitch));
	in.Position = glm::vec3(px, py, pz) * distance;

	// Update Front Direction using newly calced position
		// Subtract new position from target's position
		// Normalise result to get direction from camera to the target
	in.Front = glm::normalize(target - in.Position);

	// Update Right Direction
		// Point at a tangent to the "imaginary sphere" along its equator
			//  Calced using cross product of the Front and WorldUp vectors (90deg to both of them)
			// Normalise result to get direction from camera to pole of the sphere
	in.Right = glm::normalize(glm::cross(in.Front, in.WorldUp));

}