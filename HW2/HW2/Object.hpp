#ifndef OBJECTLOADER_HPP
#define OBJECTLOADER_HPP

#include "TextureLoader.hpp"
#include "shared.hpp"


class Object {
public:
	GLuint shaderProgram;

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

	std::vector<std::vector<GLuint>> textureMap;
	std::vector<std::vector<bool>> textureBool;
	GLuint uniformDiffuse, uniformAlpha, uniformEmissive;

	std::vector<MeshRender> data;

	Object(const char* const objFilePath, GLuint shaderProgram);
	virtual void update(float deltaTime);
	virtual void draw();
};

#endif