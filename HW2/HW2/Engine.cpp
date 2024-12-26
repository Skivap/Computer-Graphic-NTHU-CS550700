#include "Engine.hpp"
#include "TextureLoader.hpp"

/* Helper functions */
static GLuint createShaderProgram(const char** vertexSource, const char** fragmentSource);
/* Helper Variable*/
float frameBufferRect[] = {
	1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f,  1.0f,  1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f 
};

Engine::Engine() {
	// Reference: https://youtu.be/QQ3jr-9Rc1o?si=oF2wVAoJhVA6g-wH
	/* Frame Buffer */
	frameBufferProgram = createShaderProgram(&frameBufferVS, &waterFS);
	frameBuffer0Program = createShaderProgram(&frameBufferVS, &blurQuantizationDogFS);
	frameBuffer1Program = createShaderProgram(&frameBufferVS, &pixelFS);
	frameBuffer2Program = createShaderProgram(&frameBufferVS, &sinWaveFS);
	frameBuffer3Program = createShaderProgram(&frameBufferVS, &zoomFS);
	frameBuffer4Program = createShaderProgram(&frameBufferVS, &bloomFS);
	frameBufferDefaultProgram = createShaderProgram(&frameBufferVS, &frameBufferFS);
	/* JUMPSCARE BELOW */
	glUniform1i(glGetUniformLocation(frameBufferProgram,  "screenTexture"), 0);
	glUniform1i(glGetUniformLocation(frameBuffer0Program, "screenTexture"), 0);
	glUniform1i(glGetUniformLocation(frameBuffer1Program, "screenTexture"), 0);
	glUniform1i(glGetUniformLocation(frameBuffer2Program, "screenTexture"), 0);
	glUniform1i(glGetUniformLocation(frameBuffer3Program, "screenTexture"), 0);
	glUniform1i(glGetUniformLocation(frameBuffer4Program, "screenTexture"), 0);
	glUniform1i(glGetUniformLocation(frameBufferProgram,  "noiseTexture"), 0);
	glUniform1f(glGetUniformLocation(frameBuffer2Program, "offset"), 0.0f);

	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	/* Frame Buffer Texture */
	glGenTextures(1, &FBOTexture);
	glBindTexture(GL_TEXTURE_2D, FBOTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOTexture, 0);
	/* Render Buffer */
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	/* FBO VAO */
	glGenVertexArrays(1, &FBO_VAO);
	glBindVertexArray(FBO_VAO);
	glGenBuffers(1, &FBO_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, FBO_VBO);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), &frameBufferRect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	/* Noise */
	noiseTexture = loadTextureImage("noise.jpg");

	/* Shader Program */
	shaderProgram = createShaderProgram(&vertexShaderSource, &fragmentShaderSource);
	glUniform1i(glGetUniformLocation(shaderProgram, "myTexture"), 0);
	enableNormal = glGetUniformLocation(shaderProgram, "drawNormal");
	/* Object Load */
	Object * robot = new Object("lost_empire.obj", shaderProgram);
	objects.push_back(robot);

	/* GL CLEAR COLOR */
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	/* Camera */
	camera = Camera(shaderProgram);

	/* Double Check */
	auto _status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (_status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Incomplete GL Frame Buffer\n";
	}
	else {
		std::cout << "Finish Initializing\n";
	}
}

void Engine::Update(float deltaTime) {
	camera.update(deltaTime);
	for (Object* object : objects) {
		object->update(deltaTime);
	}
	dT += deltaTime;
	dT = fmod(dT, MATH_2PI);
}

/* Modify by your own preferences */
void Engine::Render() {
	/* Shader Program */
	glUseProgram(shaderProgram); // Use Shader Program

	glBindFramebuffer(GL_FRAMEBUFFER, FBO); // Enable FBO
	glUniform1i(enableNormal, menu.option7);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Color
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST); // Depth Test
	glDepthFunc(GL_LESS); // Depth Style
	camera.draw();
	for (Object * object : objects) {
		object->draw();
	}

	/* Clear */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	/* Frame Buffer Horror Code Going LTITT */
	if (menu.option1) this->RenderFrameBuffer(frameBufferProgram);
	else if (menu.option2) this->RenderFrameBuffer(frameBuffer0Program);
	else if (menu.option3) this->RenderFrameBuffer(frameBuffer1Program);
	else if (menu.option4) this->RenderFrameBuffer(frameBuffer2Program);
	else if (menu.option5) this->RenderFrameBuffer(frameBuffer3Program);
	else if (menu.option6) this->RenderFrameBuffer(frameBuffer4Program);
	else this->RenderFrameBuffer(frameBufferDefaultProgram);

	/* Finish */
	glUseProgram(0);



	/* UI */
	menu.Render();
}

void Engine::RenderFrameBuffer(GLuint program) {
	glUseProgram(program);
	glBindVertexArray(FBO_VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, FBOTexture);
	/* Noise Texture */
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	/* Uniform */
	glUniform1i(glGetUniformLocation(program, "screenLimit"), menu.slider);
	glUniform1f(glGetUniformLocation(frameBuffer2Program, "offset"), dT);
	if(onFocus) glUniform2f(glGetUniformLocation(frameBuffer3Program, "center"), 400.0f, 400.0f);
	else glUniform2f(glGetUniformLocation(frameBuffer3Program, "center"), mouseCursor.first, 800.0f - mouseCursor.second);
	/* Draw */
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

static GLuint createShaderProgram(const char** vertexSource, const char** fragmentSource) {

	GLuint vertexShader, fragmentShader, shaderProgram;

	// create vertex shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, vertexSource, NULL);
	glCompileShader(vertexShader);

	// create fragment shader
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// create shader program
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);



	// delete vertex & fragment shader
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;

}

