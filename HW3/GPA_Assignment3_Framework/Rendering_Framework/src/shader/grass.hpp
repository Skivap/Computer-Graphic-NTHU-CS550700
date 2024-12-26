#pragma once

#include "../Rendering/Shader.h"
#include "../Rendering/RendererBase.h"
#include "../Rendering/Shader.h"

#include <iostream>
#include <vector>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/material.h>

#include <filesystem>
#include <iostream>

#include <unordered_map>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Grass{
public:
	/*=========== TYPEDEF ============*/
	typedef struct _MeshData
	{
		// if OBJ preserves vertex order, you can use element array buffer for memory efficiency
		// If no data return empty vector
		std::vector<float> positions;
		std::vector<float> normals;
		std::vector<float> texcoords;
		std::vector<unsigned int> indices;
		std::vector<unsigned char> num_vertices;
		std::vector<int> material_ids; // per-face material ID
	} MeshData;

	typedef struct _MeshRender {
		GLuint vao, vbo, ebo;
		GLuint materialId, indexCount;
	} MeshRender;

	struct DrawElementsIndirectCommand {
		GLuint count;
		GLuint instanceCount;
		GLuint firstIndex;
		GLuint baseVertex;
		GLuint baseInstance;
	};

	struct InstanceProperties {
		glm::vec4 position;
	};

	/* VARIABLE */
	std::unordered_map<std::string, GLuint> textures2D;
	/* Texture */
	std::vector<std::vector<GLuint>> textureMap;
	GLuint uniformDiffuse;
	/* Render Object Data */
	std::vector<MeshRender> data;
	/* Commands */
	DrawElementsIndirectCommand drawCommands[1];
	/* SSBO */
	GLuint validInstanceDataBufferHandle;
	GLuint cmdBufferHandle;
	GLuint rawInstanceDataBufferHandle;
	/* Shader Program */
	INANOA::OPENGL::ShaderProgram* resetShader;
	INANOA::OPENGL::ShaderProgram* computeShader;

	Grass() {};


	/* TEXTURE */
	GLuint loadTextureImage(const char* path);
};