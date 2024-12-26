#include "TextureLoader.hpp"
#include "shared.hpp"

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