#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "Object.hpp"
#include "shared.hpp"
#include "Camera.hpp"
#include "UI.hpp"

class Engine {
private:
	GLuint shaderProgram, frameBufferProgram, frameBuffer0Program, 
		frameBuffer1Program, frameBuffer2Program, frameBuffer3Program, 
		frameBuffer4Program, frameBufferDefaultProgram;

	std::list<Object *> objects;

	Camera camera;

	GLuint FBO, RBO, FBOTexture, FBO_VAO, FBO_VBO;

	GLuint noiseTexture;

	UI menu;

	GLint enableNormal;

	float dT;

public:
	Engine();
	void Update(float deltaTime);
	void Render();
	void RenderFrameBuffer(GLuint program);
};

#endif