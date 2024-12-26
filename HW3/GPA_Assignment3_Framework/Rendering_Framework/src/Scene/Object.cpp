#include "Object.hpp"
#include "..\scene\Trajectory.h"


#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/material.h>

#include <filesystem>
#include <iostream>
#include <cstdlib> 

#include <unordered_map>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "SpatialSample.h"

int NUM_TOTAL_INSTANCE = 0;


INANOA::SCENE::EXPERIMENTAL::Trajectory traj;

static std::unordered_map<std::string, GLuint> textures2D;

GLuint loadTextureImage(const char* path) {

	/* If image texture already loaded, don't load it again */
	if (textures2D.find(path) != textures2D.end()) {
		return textures2D[path];
	}

	GLuint texture;

	int width;
	int height;
	int n;
	stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load(path, &width, &height, &n, 0);

	if (!data) {
		std::cerr << "Failed to load image: " << path << '\n';
		std::exit(1);
	}

	GLenum format;
	if (n == 1) format = GL_RED;
	else if (n == 3) format = GL_RGB;
	else if (n == 4) format = GL_RGBA;
	else {
		std::cerr << "Image format not supported: " << path << std::endl;
		std::exit(1);
	}

	std::cout << "Loaded image " << path << " with width: " << width << " height: " << height << " channels: " << n << std::endl;

	glGenTextures(1, &texture);
	std::cout << "teTATAF  " << texture << '\n';
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	std::cout << "Loaded Texture " << path << '\n';
	textures2D[path] = texture;
	return texture;
}

GLuint loadMultipleTextureImage(const char* path, const char* path2, const char* path3) {
	const int NUM_TEXTURE = 3;
	const int IMG_WIDTH = 1024;
	const int IMG_HEIGHT = 1024;
	const int IMG_CHANNEL = 4;
	unsigned char* textureArrayData = new unsigned char[IMG_WIDTH * IMG_HEIGHT * IMG_CHANNEL * NUM_TEXTURE];



	/* If image texture already loaded, don't load it again */
	if (textures2D.find(path) != textures2D.end()) {
		return textures2D[path];
	}

	GLuint texture;

	int width;
	int height;
	int n;
	stbi_set_flip_vertically_on_load(true);

	unsigned char* data  = stbi_load(path,  &width, &height, &n, 0);
	unsigned char* data2 = stbi_load(path2, &width, &height, &n, 0);
	unsigned char* data3 = stbi_load(path3, &width, &height, &n, 0);

	memcpy(textureArrayData, data, sizeof(unsigned char) * 1024 * 1024 * 4);
	memcpy(textureArrayData + 1024 * 1024 * 4, data2, sizeof(unsigned char) * 1024 * 1024 * 4);
	memcpy(textureArrayData + 1024 * 1024 * 4 * 2, data3, sizeof(unsigned char) * 1024 * 1024 * 4);

	if (!data) {
		std::cerr << "Failed to load image: " << path << '\n';
		std::exit(1);
	}

	std::cout << "Loaded image " << path << " with width: " << width << " height: " << height << " channels: " << n << std::endl;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

	/* todo */
	//glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, NUM_TEXTURE, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	// Subload each texture into the texture array
	//glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, textureArrayData);
	//glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, textureArrayData + 1024 * 1024 * 4);
	//glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, textureArrayData + 1024 * 1024 * 4 * 2);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 11, GL_RGBA8, IMG_WIDTH, IMG_HEIGHT, NUM_TEXTURE);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, IMG_WIDTH, IMG_HEIGHT, NUM_TEXTURE, GL_RGBA, GL_UNSIGNED_BYTE, textureArrayData);

	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	stbi_image_free(data);
	stbi_image_free(data2);
	stbi_image_free(data3);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	std::cout << "Loaded Texture " << path << '\n';
	textures2D[path] = texture;
	return texture;
}


void Object::Homework3(int vertexCount) { // init ssbo
	/* =============== Homework 3 ================ */
	/* Posittion */
	INANOA::SCENE::EXPERIMENTAL::SpatialSample* sample2 = INANOA::SCENE::EXPERIMENTAL::SpatialSample::importBinaryFile("poissonPoints_155304s.ss2");
	INANOA::SCENE::EXPERIMENTAL::SpatialSample* sample0 = INANOA::SCENE::EXPERIMENTAL::SpatialSample::importBinaryFile("poissonPoints_1010s.ss2");
	INANOA::SCENE::EXPERIMENTAL::SpatialSample* sample1 = INANOA::SCENE::EXPERIMENTAL::SpatialSample::importBinaryFile("poissonPoints_2797s.ss2");
	/* Instances */
	const int NUM_SAMPLE0 = sample0->numSample();
	const int NUM_SAMPLE1 = sample1->numSample();
	const int NUM_SAMPLE2 = sample2->numSample();
	NUM_TOTAL_INSTANCE = NUM_SAMPLE0 + NUM_SAMPLE1 + NUM_SAMPLE2;

	rawInsData = new InstanceProperties[NUM_SAMPLE0 + NUM_SAMPLE1 + NUM_SAMPLE2]; // total instances
	std::cout << "SJJ " << NUM_SAMPLE0 << " " << NUM_SAMPLE1 << " " << NUM_SAMPLE2 << '\n';
	for (int i = 0; i < NUM_SAMPLE0; i++) {
		const float * a = sample0->position(i);
		rawInsData[i].position = glm::vec4(a[0], a[1], a[2], 0.0f);
		rawInsData[i].info = glm::vec4(0);
	}
	for (int i = 0; i < NUM_SAMPLE1; i++) {
		const float* a = sample1->position(i);
		rawInsData[NUM_SAMPLE0 + i].position = glm::vec4(a[0], a[1], a[2], 1.0f);
		rawInsData[NUM_SAMPLE0 + i].info = glm::vec4(0);
	}
	for (int i = 0; i < NUM_SAMPLE2; i++) {
		const float* a = sample2->position(i);
		rawInsData[NUM_SAMPLE0 + NUM_SAMPLE1 + i].position = glm::vec4(a[0], a[1], a[2], 2.0f);
		rawInsData[NUM_SAMPLE0 + NUM_SAMPLE1 + i].info = glm::vec4(0);
	}


	/* SSBO Raw Instances Data */
	glGenBuffers(1, &rawInstanceDataBufferHandle);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, rawInstanceDataBufferHandle);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, NUM_TOTAL_INSTANCE * sizeof(InstanceProperties), rawInsData, GL_MAP_READ_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, rawInstanceDataBufferHandle);
	/* SSBO Valid Data */
	glGenBuffers(1, &validInstanceDataBufferHandle);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, validInstanceDataBufferHandle);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, NUM_TOTAL_INSTANCE * sizeof(InstanceProperties), nullptr, GL_MAP_READ_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, validInstanceDataBufferHandle);

	/* Commands */
	DrawElementsIndirectCommand drawCommands[3];
	drawCommands[0] = {
		(GLuint)indicesObject[0], 0, // Instance count
		0, 0, 0
	};
	drawCommands[1] = {
		(GLuint)indicesObject[1], 0, // Instance count
		(GLuint)indicesObject[0], 
		(GLuint)verticesObject[0],
		(GLuint)NUM_SAMPLE0
	};
	drawCommands[2] = {
		(GLuint)indicesObject[2], 0, // Instance count
		(GLuint)(indicesObject[0] + indicesObject[1]),
		(GLuint)(verticesObject[0] + verticesObject[1]),
		(GLuint)(NUM_SAMPLE0 + NUM_SAMPLE1)
	};

	glGenBuffers(1, &cmdBufferHandle);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, cmdBufferHandle);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * 3, drawCommands, GL_MAP_READ_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, cmdBufferHandle);
	//glBindBuffer(GL_DRAW_INDIRECT_BUFFER, cmdBufferHandle);
	//glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand), drawCommands, GL_STATIC_DRAW);
}

Object::Object(const char* const objFilePath, GLuint shaderProgram) {
	GLenum err;

	/* Create Programs */
	//this->shaderProgram = INANOA::OPENGL::ShaderProgram::createShaderProgram("../shader/vertexshader.glsl", "../shader/fragmentshader.glsl");
	std::filesystem::current_path("models/foliages");

	std::cout << "Importing Scene" << '\n';
	/* Scene Loader */
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		"bush01_lod2.obj",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType
	);

	Assimp::Importer importer2, importer3;
	const aiScene* scene2 = importer2.ReadFile(
		"bush05_lod2.obj",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType
	);

	const aiScene* scene3 = importer3.ReadFile(
		"grassB.obj",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType
	);

	// std::filesystem::current_path("../..");
	if (!scene ){// || !scene2 || !scene3) {
		std::cerr << "Failed To load Scene" << '\n';
		std::cerr << importer.GetErrorString() << '\n';
		int tmp;
		std::cin >> tmp;
		std::exit(1);
	}
	if (!scene2) {// || !scene2 || !scene3) {
		std::cerr << "Failed To load Scene" << '\n';
		std::cerr << importer2.GetErrorString() << '\n';
		int tmp;
		std::cin >> tmp;
		std::exit(1);
	}
	if (!scene3) {// || !scene2 || !scene3) {
		std::cerr << "Failed To load Scene" << '\n';
		std::cerr << importer3.GetErrorString() << '\n';
		int tmp;
		std::cin >> tmp;
		std::exit(1);
	}

	std::cout << "Load Model with Object: " << scene->mNumMeshes << " and Materials: " << scene->mNumMaterials << std::endl;
	std::vector<MeshData> meshes;
	
	/* Iterate every Mesh in the File */
	{
		int i = 0;

		//std::cout << i << ": " << aimesh->mName.data << '\n';
		MeshData mesh;

		std::vector<aiMesh*> meshesss = { scene->mMeshes[i], scene2->mMeshes[i], scene3->mMeshes[i] };

		for(aiMesh* aimesh : meshesss)
		{
			int index_offset = 0;
			for (int f = 0; f < aimesh->mNumFaces; f++) {
				auto face = aimesh->mFaces[f];
				int fv = aimesh->mFaces[f].mNumIndices;

				for (int v = 0; v < fv; v++) {
					int idx = face.mIndices[v];
					/* Vertex */
					mesh.positions.push_back(aimesh->mVertices[idx][0]);
					mesh.positions.push_back(aimesh->mVertices[idx][1]);
					mesh.positions.push_back(aimesh->mVertices[idx][2]);
					/* UV */
					mesh.texcoords.push_back(aimesh->mTextureCoords[0][idx][0]);
					mesh.texcoords.push_back(aimesh->mTextureCoords[0][idx][1]);
					mesh.texcoords.push_back(indicesObject.size());
					/* Normals */
					mesh.normals.push_back(aimesh->mNormals[idx][0]);
					mesh.normals.push_back(aimesh->mNormals[idx][1]);
					mesh.normals.push_back(aimesh->mNormals[idx][2]);

					mesh.indices.push_back(index_offset + v);
				}

				mesh.material_ids.push_back(aimesh->mMaterialIndex);
				index_offset += fv;
			}

			indicesObject.push_back(mesh.indices.size() / 3);
			verticesObject.push_back(mesh.positions.size() / 3);
		}
		indicesObject[2] -= indicesObject[1];
		indicesObject[1] -= indicesObject[0];
		verticesObject[2] -= verticesObject[1];
		verticesObject[1] -= verticesObject[0];
		
		meshes.push_back(mesh);
	}
	std::cout << "VERTICESS " << verticesObject[0] << " " << verticesObject[1] << " " << verticesObject[2] << '\n';
	std::cout << "INDICeSSS " << indicesObject[0] << " " << indicesObject[1] << " " << indicesObject[2] << '\n';

	std::cout << "Finished loading mesh, now compiling it..." << '\n';

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		MeshRender temp_data;
		glGenVertexArrays(1, &temp_data.vao); //Generate vao
		glBindVertexArray(temp_data.vao); //Bind vao

		glGenBuffers(1, &temp_data.vbo); //Generate vbo
		glBindBuffer(GL_ARRAY_BUFFER, temp_data.vbo); //Bind vbo

		size_t positionSize = meshes[i].positions.size() * sizeof(float);
		size_t textureSize = meshes[i].texcoords.size() * sizeof(float);
		size_t normalsSize = meshes[i].normals.size() * sizeof(float);
		size_t indicesSize = meshes[i].indices.size() * sizeof(unsigned int);

		

		glBufferData(GL_ARRAY_BUFFER, (positionSize + textureSize + normalsSize + sizeof(rawInsData)), NULL, GL_STATIC_DRAW); //Get vbo space
		//				GL ARRAY BUFFER | INIT						| SIZE			| ARRAY
		glBufferSubData(GL_ARRAY_BUFFER, 0, positionSize, meshes[i].positions.data());
		glBufferSubData(GL_ARRAY_BUFFER, positionSize, textureSize, meshes[i].texcoords.data());
		glBufferSubData(GL_ARRAY_BUFFER, positionSize + textureSize, normalsSize, meshes[i].normals.data()); //Load data into buffer

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); //Set attribute of data
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(positionSize));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + textureSize));
		glEnableVertexAttribArray(2);

		glGenBuffers(1, &temp_data.ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, temp_data.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, meshes[i].indices.data(), GL_STATIC_DRAW);

		Homework3(meshes[i].positions.size());

		/* HOMEWORK 3 */
		glBindBuffer(GL_ARRAY_BUFFER, validInstanceDataBufferHandle);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)(0));
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, 1);
		/* End of HW 3*/

		temp_data.materialId = scene->mMeshes[i]->mMaterialIndex;
		temp_data.indexCount = meshes[i].indices.size();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		data.push_back(temp_data);
	}

	/* Material loading 
	if (scene->mNumMaterials == 0) {
		std::cout << "No Material Found, using sample texture ..." << '\n';
		GLuint texture = loadTextureImage("sample.jpg");
		textureMap.push_back({ texture, 0, 0 });
		textureBool.push_back({ false, false, false });

		for (auto& tmp : data) {
			tmp.materialId = 0;
		}
	}
	else */

	{ 
		int i = 0;
		
		aiMaterial* material = scene->mMaterials[0];
		aiMaterial* material2 = scene2->mMaterials[0];
		aiMaterial* material3 = scene3->mMaterials[0];
		int idx = 0;
		aiString path, path2, path3;

		GLuint diffuseTexture = 0;
		const char* texturePath = "bush01.png";
		const char* texturePath2 = "bush05.png";
		const char* texturePath3 = "grassB_albedo.png";

		/*
		if (material->GetTexture(aiTextureType_DIFFUSE, idx, &path) == AI_SUCCESS) {
			texturePath = path.data;
		}
		if (material2->GetTexture(aiTextureType_DIFFUSE, idx, &path2) == AI_SUCCESS) {
			texturePath2 = path2.data;
		}
		if (material3->GetTexture(aiTextureType_DIFFUSE, idx, &path3) == AI_SUCCESS) {
			texturePath3 = path3.data;
		} */
		uniformDiffuse = glGetUniformLocation(shaderProgram, "texture2D");
		slimeTextureUniform = glGetUniformLocation(shaderProgram, "textureSlime");
		glUseProgram(shaderProgram);

		glUniform1i(uniformDiffuse, 0);
		glUniform1i(slimeTextureUniform, 1);
		diffuseTexture = loadMultipleTextureImage(texturePath, texturePath2, texturePath3);

		textureMap.push_back({ diffuseTexture });
	}


	this->shaderProgram = shaderProgram;
	Slime();

	std::cout << "Finish Compiling Object with data  length: " << data.size() << '\n';
	std::filesystem::current_path("../..");



	resetShader = INANOA::OPENGL::ShaderProgram::createShaderProgramForComputeShader("src/shader/resetShader.glsl");
	computeShader = INANOA::OPENGL::ShaderProgram::createShaderProgramForComputeShader("src/shader/computeShader.glsl");

	//GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "=======================" << '\n';
		std::cerr << "OpenGL error: " << err << '\n';
		std::cerr << "=======================" << '\n';
	}

}

void Object::Slime() {
	std::cout << "Importing Scene" << '\n';
	/* Scene Loader */
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		"slime.obj",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (!scene) {
		std::cerr << "Failed To load Scene" << '\n';
		std::cerr << importer.GetErrorString() << '\n';
		std::exit(1);
	}

	std::cout << "Load Model with Object: " << scene->mNumMeshes << " and Materials: " << scene->mNumMaterials << std::endl;
	std::vector<MeshData> meshes;

	/* Iterate every Mesh in the File */



	for (int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* aimesh = scene->mMeshes[i];
		std::cout << i << ": " << aimesh->mName.data << '\n';
		MeshData mesh;
		int index_offset = 0;

		for (int f = 0; f < aimesh->mNumFaces; f++) {
			auto face = aimesh->mFaces[f];
			int fv = aimesh->mFaces[f].mNumIndices;

			for (int v = 0; v < fv; v++) {
				int idx = face.mIndices[v];
				/* Vertex */
				mesh.positions.push_back(aimesh->mVertices[idx][0]);
				mesh.positions.push_back(aimesh->mVertices[idx][1]);
				mesh.positions.push_back(aimesh->mVertices[idx][2]);
				/* UV */
				mesh.texcoords.push_back(aimesh->mTextureCoords[0][idx][0]);
				mesh.texcoords.push_back(aimesh->mTextureCoords[0][idx][1]);
				mesh.texcoords.push_back(0);
				/* Normals */
				mesh.normals.push_back(aimesh->mNormals[idx][0]);
				mesh.normals.push_back(aimesh->mNormals[idx][1]);
				mesh.normals.push_back(aimesh->mNormals[idx][2]);

				mesh.indices.push_back(index_offset + v);
			}

			mesh.material_ids.push_back(aimesh->mMaterialIndex);
			index_offset += fv;
		}

		meshes.push_back(mesh);
	}

	std::cout << "Finished loading mesh, now compiling it..." << '\n';


	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		MeshRender temp_data;
		glGenVertexArrays(1, &temp_data.vao); //Generate vao
		glBindVertexArray(temp_data.vao); //Bind vao

		glGenBuffers(1, &temp_data.vbo); //Generate vbo
		glBindBuffer(GL_ARRAY_BUFFER, temp_data.vbo); //Bind vbo

		size_t positionSize = meshes[i].positions.size() * sizeof(float);
		size_t textureSize = meshes[i].texcoords.size() * sizeof(float);
		size_t normalsSize = meshes[i].normals.size() * sizeof(float);
		size_t indicesSize = meshes[i].indices.size() * sizeof(unsigned int);

		glBufferData(GL_ARRAY_BUFFER, (positionSize + textureSize + normalsSize), NULL, GL_STATIC_DRAW); //Get vbo space
		//				GL ARRAY BUFFER | INIT						| SIZE			| ARRAY
		glBufferSubData(GL_ARRAY_BUFFER, 0, positionSize, meshes[i].positions.data());
		glBufferSubData(GL_ARRAY_BUFFER, positionSize, textureSize, meshes[i].texcoords.data());
		glBufferSubData(GL_ARRAY_BUFFER, positionSize + textureSize, normalsSize, meshes[i].normals.data()); //Load data into buffer

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); //Set attribute of data
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(positionSize));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + textureSize));
		glEnableVertexAttribArray(2);

		glGenBuffers(1, &temp_data.ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, temp_data.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, meshes[i].indices.data(), GL_STATIC_DRAW);

		temp_data.materialId = scene->mMeshes[i]->mMaterialIndex;
		temp_data.indexCount = meshes[i].indices.size();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		slime = temp_data;
	}

	{
		aiMaterial* material = scene->mMaterials[0];
		int idx = 0;
		aiString path;

		
			const char* texturePath = "slime_albedo.jpg";
			slimeTexture = loadTextureImage(texturePath);
		
	}

	std::cout << "SLime tex " << slimeTexture << '\n';

	slimePos = glGetUniformLocation(shaderProgram, "slimePos");
	//glActiveTexture(GL_TEXTURE1);

	slimePosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	traj.enable(true);// = true;
}

void Object::update(float deltaTime) {
	/*
	float randomX = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 0.8f;
	float randomZ = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 0.8f;

	slimePosition += glm::vec4(randomX, 0.0f, randomZ, 0.0f) * deltaTime * 10.0f; */
	traj.update();
	slimePosition = traj.positionVec4();
}

void Object::draw(INANOA::OPENGL::RendererBase* m_renderer, glm::mat4 mat, bool drawCulling) {
	if (drawCulling) {
		/* RESET CS PROGRAM */
		resetShader->useProgram();
		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		/* CULLING */
		computeShader->useProgram();
		// send the necessary information to compute shader (must after useProgram)
		static unsigned int uniformInstance = glGetUniformLocation(computeShader->programId(), "numMaxInstance");
		static unsigned int uniformProjView = glGetUniformLocation(computeShader->programId(), "viewProjMat");
		static unsigned int uniformSlime = glGetUniformLocation(computeShader->programId(), "slimePos");

		glUniform4f(uniformSlime, slimePosition.x, slimePosition.y, slimePosition.z, slimePosition.w);
		glUniform1i(uniformInstance, NUM_TOTAL_INSTANCE);
		glUniformMatrix4fv(uniformProjView, 1, false, glm::value_ptr(mat));
		// start GPU process
		glDispatchCompute(NUM_TOTAL_INSTANCE / 1024, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//INANOA::OPENGL::RendererBase* renderer = new INANOA::OPENGL::RendererBase();
	glUseProgram(shaderProgram);

	glUniformMatrix4fv(0, 1, false, glm::value_ptr(glm::mat4(1.0f)));

	{
		glUniform4f(slimePos, 0.0f, 0.0f, 0.0f, 0.0f);

		MeshRender& object = data[0];

		/* Texture */
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureMap[0][0]);
		glUniform1i(uniformDiffuse, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, slimeTexture);
		glUniform1i(slimeTextureUniform, 1);
		/*
		glUniform1i(glGetUniformLocation(shaderProgram, "diffuseMap"), 0);
		glActiveTexture(GL_TEXTURE1); // Alpha Texture
		glBindTexture(GL_TEXTURE_2D, textureMap[object.materialId][1]);
		glUniform1i(glGetUniformLocation(shaderProgram, "alphaMap"), 1);
		glActiveTexture(GL_TEXTURE2); // Emmisive Texture
		glBindTexture(GL_TEXTURE_2D, textureMap[object.materialId][2]);
		glUniform1i(glGetUniformLocation(shaderProgram, "emissiveMap"), 2);

		glUniform1i(uniformDiffuse, textureBool[object.materialId][0]);
		glUniform1i(uniformAlpha, textureBool[object.materialId][1]);
		glUniform1i(uniformEmissive, textureBool[object.materialId][2]); 
		*/

		/* Bind */
		glBindVertexArray(object.vao);

		/* Templete to add: Set Uniform */
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, cmdBufferHandle);
		/* TODO: DRAW */
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)0, 3, 0);


		glBindVertexArray(0);
	}
	{

		glUniform4f(slimePos, slimePosition.x, slimePosition.y, slimePosition.z, slimePosition.w);

		m_renderer->setShadingModel(INANOA::OPENGL::ShadingModelType::SLIME);

		glBindVertexArray(slime.vao);
		glDrawElements(GL_TRIANGLES, slime.indexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	glUniform4f(slimePos, 0.0f, 0.0f, 0.0f, 0.0f);
}