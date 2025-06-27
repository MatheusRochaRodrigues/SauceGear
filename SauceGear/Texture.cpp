#include "Texture.h" 

SrcTexture::SrcTexture(const char* path, const char* texType, GLuint slot, bool SpaceCorSRGB) {
	type = texType;

	glGenTextures(1, &ID);

	int width, height, nrChannels;
	unsigned char* bytes = stbi_load(path, &width, &height, &nrChannels, 0);
	if (bytes) {

		GLenum format;
		if (nrChannels == 1) {
			format = GL_RED;
		}
		else if (nrChannels == 3) {
			format = GL_RGB;
		}
		else if (nrChannels == 4) {
			format = GL_RGBA;
		}

		//FormatIntern
		GLenum format_intern;
		if (SpaceCorSRGB) {
			if (nrChannels == 3) {
				format_intern = GL_SRGB;
			}
			else if (nrChannels == 4) {
				format_intern = GL_SRGB_ALPHA;
			}
		}

		glActiveTexture(GL_TEXTURE0 + slot);
		unit = slot;
		glBindTexture(GL_TEXTURE_2D, ID);

		if (SpaceCorSRGB) {
			glTexImage2D(GL_TEXTURE_2D, 0, format_intern, width, height, 0, format, GL_UNSIGNED_BYTE, bytes);
		}
		else {
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, bytes);
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(bytes);
	}
	else {
		std::cout << "Texture Failed to load at path: " << path << std::endl;
		stbi_image_free(bytes);
	}

	formatTex = GL_TEXTURE_2D;
}



// Texture for Allocate Memory  
SrcTexture::SrcTexture(
	unsigned int width,
	unsigned int height,
	GLenum internalFormat,
	OPERATION_TEX choise,
	GLenum format,
	unsigned int samples
) {

	glGenTextures(1, &ID);

	if (choise == TEX_DEFAULT) {
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			internalFormat,
			width,
			height,
			0,
			format == -10 ? internalFormat : format,
			GL_UNSIGNED_BYTE,
			NULL
		); //Warning


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Default Time
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Default Time

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //WARNING TEMPORARIO
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); //WARNING TEMPORARIO


		glBindTexture(GL_TEXTURE_2D, 0);

		formatTex = GL_TEXTURE_2D;
		type = "null_Default";

	}
	else if (choise == TEX_MULTI_SAMPLES) {
		// __Allocate Memory Buffer for Texture MultiSamples of Frame Buffers

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ID);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_TRUE);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		formatTex = GL_TEXTURE_2D_MULTISAMPLE;
		type = "null_MultiSample";
	}
}

// Texture BackRender


// LOAD CUBE MAP
//	unsigned int loadCubeMap(std::vector<std::string> Faces)
SrcTexture::SrcTexture(std::vector<std::string> Faces) {
	//unsigned int textureID;
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < Faces.size(); i++) {
		unsigned char* data = stbi_load(Faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
				GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Failed to load Texture CubeMap at path: " << Faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//return textureID;
}


void SrcTexture::TexCubeMap(unsigned int WIDTH, unsigned int HEIGHT, unsigned int round) {
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	for (unsigned int i = 0; i < 6; ++i) {
		//	GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


}



// __Operations with Texture
void SrcTexture::tex_Unit(Shader& shader, const char* uniform, GLuint unit) {
	GLuint TEXuni = glGetUniformLocation(shader.ID, uniform);
	//shader.Activate();	WARNING TEMP
	glUniform1i(TEXuni, unit);
}

void SrcTexture::Bind() {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(formatTex, ID); //GL_TEXTURE_2D
}

void SrcTexture::Unbind() {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(formatTex, 0); //GL_TEXTURE_2D
}

void SrcTexture::Delete() {
	glDeleteTextures(1, &ID);
}



//Back Texture
void SrcTexture::RenderBackBufferTexture(unsigned int WIDTH, unsigned int HEIGHT, GLenum specBack, GLint renderInterBackTex, GLenum format) {
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);

	glTexImage2D
	(
		GL_TEXTURE_2D,
		0,
		specBack,
		WIDTH,
		HEIGHT,
		0,
		format == NULL ? GL_DEPTH_STENCIL : format,
		renderInterBackTex,
		NULL
	);

	if (format == NULL) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Default Time
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Default Time
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);		//temp
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);		//temp
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);
	}


	glBindTexture(GL_TEXTURE_2D, 0);

	formatTex = GL_TEXTURE_2D;
	type = "depthTex";
};
	 