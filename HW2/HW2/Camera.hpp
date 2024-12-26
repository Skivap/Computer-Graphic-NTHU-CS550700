#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "shared.hpp"

class Camera
{
private:
	/* Uniform */
	GLuint uniformView;
	GLuint uniformProj;
	GLuint program;
	/* View */
	glm::mat4 view;
	glm::mat4 projection;
	/* Camera Position & Angle */
	glm::vec3 position;
	glm::vec3 orientation;
	/* Rotation */
	float rotX, rotY;

public:
	Camera();
	Camera(GLuint program);
	void update(float deltaTime);
	void draw();
};

#endif