#include "Object.hpp"

// Reference: https://www.mbsoftworks.sk/tutorials/opengl3/20-assimp-model-loading/ + Tiny Obj Loader

Object::Object(const char* const objFilePath, GLuint shaderProgram) {
	std::cout << "Importing Scene" << '\n';
	/* Scene Loader */
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(objFilePath,
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
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(positionSize));
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

		data.push_back(temp_data);
	}

	/* Material loading */;
	if (scene->mNumMaterials == 0) {
		std::cout << "No Material Found, using sample texture ..." << '\n';
		GLuint texture = loadTextureImage("sample.jpg");
		textureMap.push_back({ texture, 0, 0 });
		textureBool.push_back({ false, false, false });

		for (auto& tmp : data) {
			tmp.materialId = 0;
		}
	}
	else for (int i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* material = scene->mMaterials[i];
		int idx = 0;
		aiString path;

		GLuint diffuseTexture = 0, alphaTexture = 0, EmissiveTexture = 0;
		bool hasDiffuse = false, hasAlpha = false, hasEmissive = false;

		if (material->GetTexture(aiTextureType_DIFFUSE, idx, &path) == AI_SUCCESS) {
			const char * texturePath = path.data;
			diffuseTexture = loadTextureImage(texturePath);
			hasDiffuse = true;
		}
		if (material->GetTexture(aiTextureType_OPACITY, idx, &path) == AI_SUCCESS) {
			const char* texturePath = path.data;
			alphaTexture = loadTextureImage(texturePath);
			hasAlpha = true;
		}
		if (material->GetTexture(aiTextureType_EMISSIVE, idx, &path) == AI_SUCCESS) {
			const char* texturePath = path.data;
			EmissiveTexture = loadTextureImage(texturePath);
			hasEmissive = true;
		}

		textureMap.push_back({ diffuseTexture, alphaTexture, EmissiveTexture });
		textureBool.push_back({ hasDiffuse, hasAlpha, hasEmissive });
	}

	uniformDiffuse = glGetUniformLocation(shaderProgram, "hasDiffuseMap");
	uniformAlpha = glGetUniformLocation(shaderProgram, "hasAlphaMap");
	uniformEmissive = glGetUniformLocation(shaderProgram, "hasEmissiveMap");

	this->shaderProgram = shaderProgram;

	std::cout << "Finish Compiling Object with data  length: " << data.size() << '\n';

}

void Object::update(float deltaTime) {

}

void Object::draw() {

	for (int i = 0; i<data.size(); i++) {

		if (i == 5) continue;

		MeshRender& object = data[i];
		
		/* Texture*/
		glActiveTexture(GL_TEXTURE0); // Diffuse Texture
		glBindTexture(GL_TEXTURE_2D, textureMap[object.materialId][0]);
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

		/* Bind */
		glBindVertexArray(object.vao);

		/* Templete to add: Set Uniform */

		/* TODO: DRAW */
		glDrawElements(GL_TRIANGLES, object.indexCount, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}
	for (int i = 5; i == 5; i++) {
		MeshRender& object = data[i];

		/* Texture*/
		glActiveTexture(GL_TEXTURE0); // Diffuse Texture
		glBindTexture(GL_TEXTURE_2D, textureMap[object.materialId][0]);
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

		/* Bind */
		glBindVertexArray(object.vao);

		/* Templete to add: Set Uniform */

		/* TODO: DRAW */
		glDrawElements(GL_TRIANGLES, object.indexCount, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}
}