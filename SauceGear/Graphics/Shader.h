#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include<glad/glad.h> // holds all OpenGL type declarations
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>
#include<vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::string get_file_contents(const char* filename);


class Shader
{
public:
	// Reference ID of the Shader Program
	GLuint ID;

	std::string name;

	Shader() = default;

	// Constructor that build the Shader Program from 2 different shaders
	Shader(const char* vertexFile, const char* fragmentFile, const std::vector<std::pair<std::string, int>>& defines = {});
	// Constructor that build geometry shader too
	Shader(const char* vertexFile, const char* geometryFile, const char* fragmentFile, const std::vector<std::pair<std::string, int>>& defines = {});

	void ReloadWithDefines(const std::vector<std::pair<std::string, int>>& defines);

	// Activates the Shader Program
	void use();
	// Deletes the Shader Program
	void Delete();

	void setMat4(const std::string& name, glm::mat4 matrix);
	void setMat3(const std::string& name, glm::mat3 matrix);

	void setVec2(const std::string& name, float v1, float v2);
	void setVec2(const std::string& name, glm::vec2 vector);

	void setVec3(const std::string& name, float v1, float v2, float v3);
	void setVec3(const std::string& name, glm::vec3 vector);
	void setVec4(const std::string& name, float v1, float v2, float v3, float v4);
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;

	// Arrays
	void setIntArray(const std::string& name, int* values, int count);

	// Textures Arrays
	void setTexture2D(const std::string& name, GLuint texID, GLenum unit) const;
	void setTexture2DArray(const std::string& name, GLuint texID, GLenum unit) const;
	void setTextureCube(const std::string& name, GLuint texID, GLenum unit) const;
	void setTextureCubeArray(const std::string& name, GLuint texID, GLenum unit) const;

private:
	// Aqui estava o problema: a função precisa estar declarada dentro da classe!
	void compileErrors(unsigned int shader, const char* type);

	const char* vertexFile;
	const char* geometryFile;
	const char* fragmentFile;

	std::string ShaderPathDefault = "Resources/Shaders/";
};


#endif