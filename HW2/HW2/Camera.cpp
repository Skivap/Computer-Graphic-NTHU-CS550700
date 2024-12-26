#include "Camera.hpp"

// Reference: https://youtu.be/86_pQCKOIPk?si=iska82d2fliSDMZ5
static const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
static float speed = 10.0f;
const float myPOV = 45.0f;
const float halfPOV = 22.5f;

Camera::Camera() { }

Camera::Camera(GLuint program) {
	this->program = program;
	uniformView = glGetUniformLocation(program, "view");
	uniformProj = glGetUniformLocation(program, "proj");
	position = glm::vec3(-40.0f, 20.0f, 0.0f);
	orientation = glm::vec3(1.0f, 0.0f, 0.0f);
	rotX = rotY = 0.0f;
}

void Camera::update(float deltaTime) {

	/* Keyboard Function */
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += deltaTime * orientation * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= deltaTime * orientation * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += deltaTime * glm::normalize(glm::cross(orientation, up)) * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= deltaTime * glm::normalize(glm::cross(orientation, up)) * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		position += deltaTime * up * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		position -= deltaTime * up * speed;
	}

	/* Mouse Function */
	static bool firstClick = true;
	static float sens = 1.0f;
	static std::pair<double, double> lastCur;
	if(onFocus){ 
		if (firstClick) {
			lastCur = mouseCursor;
			firstClick = false;
		}
		else {
			float dX = static_cast<float>(mouseCursor.first - lastCur.first);
			float dY = static_cast<float>(mouseCursor.second - lastCur.second);
			lastCur = mouseCursor;

			rotX += dX * deltaTime * sens;
			rotY += dY * deltaTime * sens;
			rotX = fmod(rotX, MATH_2PI);
			rotY = glm::clamp<float>(rotY, glm::radians(-89.0f), glm::radians(89.0f));
			orientation = glm::vec3(cos(rotX) * cos(rotY), -sin(rotY), sin(rotX) * cos(rotY));
		}
	}
	else {
		firstClick = true;
	}
}

void Camera::draw() {
	/* Uniform Proj + View */
	view = glm::lookAt(
		position, // Eye Location
		position + orientation, // Destiny
		up // Up
	);
	projection = glm::perspective(glm::radians(myPOV), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(uniformProj, 1, GL_FALSE, glm::value_ptr(projection));
}