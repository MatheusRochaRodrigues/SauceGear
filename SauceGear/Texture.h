#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include <glad/glad.h>
#include <stb/stb_image.h>
#include "Shader.h"
#include <stdlib.h>
#include <vector>

using namespace std;


//struct Texture {
//	unsigned int id;
//	string type;
//	string path;
//};


enum OPERATION_TEX {
	TEX_DEFAULT,
	TEX_MULTI_SAMPLES
};

class SrcTexture {
public:
	unsigned int ID;
	const char* type;
	GLenum formatTex;
	GLuint unit;

	//SrcTexture() = default;
	SrcTexture() = default;

	// __Create texture with image
	SrcTexture(const char* path, const char* texType, GLuint slot, bool SpaceCorSRGB); // SRGB
	//	SrcTexture(const char* path, const char* texType, GLuint slot); // Normal


// __Create Buffer Texture for allocate space in memory for FrameBuffer
	SrcTexture(
		unsigned int width,
		unsigned int height,
		GLenum internalFormat = GL_RGB,
		OPERATION_TEX choise = TEX_DEFAULT,
		GLenum format = -10,
		unsigned int samples = 0  // MultiSamples
	);


	SrcTexture(
		std::vector<std::string> Faces
	);

	void TexCubeMap(unsigned int WIDTH, unsigned int HEIGHT, unsigned int round = 6);


	void RenderBackBufferTexture(unsigned int WIDTH, unsigned int HEIGHT, GLenum specBack, GLint renderInterBackTex, GLenum format = NULL);


	// Handling texture created 
	void tex_Unit(Shader& shader, const char* uniform, GLuint unit);
	void Bind();
	void Unbind();
	void Delete();
};
	 
#endif
