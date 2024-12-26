#ifndef SHARED_HPP
#define SHARED_HPP

/* IMGUI */
#include "dependencies/imgui/imgui.h"
#include "dependencies/imgui/imgui_impl_glfw.h"
#include "dependencies/imgui/imgui_impl_opengl3.h"

/* GLFW / GLAD */
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL

/* GLM */
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/type_ptr.hpp>

/* Assimp */
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/material.h>

/* Other Third Party Libraries */
#include <tiny_obj_loader.h>
#include <stb/stb_image.h>

/* Standard Library */
#include <iostream>
#include <cstdlib>
#include <string>
#include <string.h>
#include <vector>
#include <list>
#include <cstdint>
#include <iomanip>
#include <unordered_map>
#include <algorithm>
#include <filesystem>

/* Shared variables */
extern GLFWwindow* window;

extern const char* vertexShaderSource;
extern const char* fragmentShaderSource;
extern const char* frameBufferVS;
extern const char* frameBufferFS;
/* Post Processing Filter */
extern const char* blurFS;
extern const char* quantizationFS;
extern const char* diffGaussianFS;
extern const char* blurQuantizationDogFS;
extern const char* waterFS;
extern const char* pixelFS;
extern const char* sinWaveFS;
extern const char* zoomFS;
extern const char* bloomFS;

extern const float MAX_FPS;
extern const int SCREEN_HEIGHT;
extern const int SCREEN_WIDTH;
extern std::pair<double, double> mouseCursor;

extern unsigned int _textureCounter;

extern bool onFocus;

/* DEFINE */
#define MATH_PI 3.14159265358979323846
#define MATH_2PI 6.28318530717958647692

#endif