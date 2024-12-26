#pragma once

#include "../Rendering/Shader.h"
#include "../Rendering/RendererBase.h"
#include "../Rendering/Shader.h"

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	glm::vec4 info;
};

class Object {
public:

	std::vector<std::vector<GLuint>> textureMap;
	std::vector<std::vector<bool>> textureBool;
	GLuint uniformDiffuse, uniformAlpha, uniformEmissive;

	std::vector<MeshRender> data;
	MeshRender slime;

	/* IDK WHAT IS GOING ON 
	GLuint resetCSProgramHandle;
	GLuint cullingCSProgramHandle;
	GLuint renderProgramHandle; */

	Object(){
		
	}
	Object(const char* const objFilePath, GLuint shaderProgram);
	virtual void update(float deltaTime);
	virtual void draw(INANOA::OPENGL::RendererBase* m_renderer, glm::mat4 mat, bool drawCulling);
	void Homework3(int vertexCount);
	void Slime();

	GLuint validInstanceDataBufferHandle; 	
	GLuint cmdBufferHandle; 
	GLuint rawInstanceDataBufferHandle;

	InstanceProperties* rawInsData;

	INANOA::OPENGL::ShaderProgram* resetShader;
	INANOA::OPENGL::ShaderProgram* computeShader;
	INANOA::OPENGL::RendererBase* m_renderer;

	std::vector<int> indicesObject;
	std::vector<int> verticesObject;

	GLuint shaderProgram;

	GLuint slimeTexture;
	GLuint slimeTextureUniform;
	GLuint slimePos;
	glm::vec4 slimePosition;

};
