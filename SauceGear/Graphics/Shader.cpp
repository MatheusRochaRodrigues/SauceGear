#include"Shader.h"
#include <regex> 

// Reads a text file and outputs a string with everything in the text file
std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary);
	if (!in)
		throw std::runtime_error(std::string("\n<Error> To open archive Shader : ") + filename);
		//throw(errno);

	std::string contents;
	in.seekg(0, std::ios::end);
	contents.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();
	return contents;
}  

std::string UpdateDefine(const std::string& shaderCode, const std::vector<std::pair<std::string, int>>& defines) {
	std::string code = shaderCode;

	for (auto& def : defines) {
		const std::string& name = def.first;
		int value = def.second;

		std::regex defineRegex("#define\\s+" + name + "\\s+\\d+");
		if (std::regex_search(code, defineRegex)) {
			// Substitui o valor existente
			code = std::regex_replace(code, defineRegex, "#define " + name + " " + std::to_string(value));
		}
		else {
			// Adiciona o define no topo
			code = "#define " + name + " " + std::to_string(value) + "\n" + code;
		}
	}

	return code;
} 

// Constructor that build the Shader Program from 2 different shaders
Shader::Shader(const char* vertexFile, const char* fragmentFile, const std::vector<std::pair<std::string, int>>& defines)
{
	this->vertexFile = vertexFile;
	this->geometryFile = NULL;
	this->fragmentFile = fragmentFile;

	name = vertexFile; 
	std::string vPath = ShaderPathDefault + vertexFile;
	std::string fPath = ShaderPathDefault + fragmentFile;

	try {
		vertexFile = vPath.c_str(); 
		fragmentFile = fPath.c_str();

		// Read vertexFile and fragmentFile and store the strings
		std::string vertexCode = get_file_contents(vertexFile);
		std::string fragmentCode = get_file_contents(fragmentFile);

		// injeta defines
		vertexCode = UpdateDefine(vertexCode, defines);
		fragmentCode = UpdateDefine(fragmentCode, defines);

		// Convert the shader source strings into character arrays
		const char* vertexSource = vertexCode.c_str();
		const char* fragmentSource = fragmentCode.c_str();

		// Create Vertex Shader Object and get its reference
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		// Attach Vertex Shader source to the Vertex Shader Object
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		// Compile the Vertex Shader into machine code
		glCompileShader(vertexShader);
		// Checks if Shader compiled succesfully
		compileErrors(vertexShader, "VERTEX");

		// Create Fragment Shader Object and get its reference
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		// Attach Fragment Shader source to the Fragment Shader Object
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		// Compile the Vertex Shader into machine code
		glCompileShader(fragmentShader);
		// Checks if Shader compiled succesfully
		compileErrors(fragmentShader, "FRAGMENT");

		// Create Shader Program Object and get its reference
		ID = glCreateProgram();
		// Attach the Vertex and Fragment Shaders to the Shader Program
		glAttachShader(ID, vertexShader);
		glAttachShader(ID, fragmentShader);
		// Wrap-up/Link all the shaders together into the Shader Program
		glLinkProgram(ID);
		// Checks if Shaders linked succesfully
		compileErrors(ID, "PROGRAM");

		// Delete the now useless Vertex and Fragment Shader objects
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}
	catch (const std::exception& e) {
		std::cerr << "[Error] - To Create Shader (" << vPath << ", " << fPath << "):\n" << e.what() << "\n\n";
	}
}

// Constructor that build the Shader Program from 2 different shaders
Shader::Shader(const char* vertexFile, const char* geometryFile, const char* fragmentFile, const std::vector<std::pair<std::string, int>>& defines)
{
	this->vertexFile = vertexFile;
	this->geometryFile = geometryFile;
	this->fragmentFile = fragmentFile; 

	name = vertexFile; 
	std::string vPath = ShaderPathDefault + vertexFile;
	std::string gPath = ShaderPathDefault + geometryFile;
	std::string fPath = ShaderPathDefault + fragmentFile;

	try {
		vertexFile = vPath.c_str();
		geometryFile = gPath.c_str();
		fragmentFile = fPath.c_str();

		// Read vertexFile and fragmentFile and store the strings
		std::string vertexCode = get_file_contents(vertexFile);
		std::string geometryCode = get_file_contents(geometryFile);
		std::string fragmentCode = get_file_contents(fragmentFile);

		// Convert the shader source strings into character arrays
		const char* vertexSource = vertexCode.c_str();
		const char* geometrySource = geometryCode.c_str();
		const char* fragmentSource = fragmentCode.c_str();

		// Create Vertex Shader Object and get its reference
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		// Attach Vertex Shader source to the Vertex Shader Object
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		// Compile the Vertex Shader into machine code
		glCompileShader(vertexShader);
		// Checks if Shader compiled succesfully
		compileErrors(vertexShader, "VERTEX");

		// Create Geometry Shader
		GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &geometrySource, NULL);
		glCompileShader(geometryShader);
		//Check Error
		compileErrors(geometryShader, "GEOMETRY");

		// Create Fragment Shader Object and get its reference
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		// Attach Fragment Shader source to the Fragment Shader Object
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		// Compile the Vertex Shader into machine code
		glCompileShader(fragmentShader);
		// Checks if Shader compiled succesfully
		compileErrors(fragmentShader, "FRAGMENT");

		// Create Shader Program Object and get its reference
		ID = glCreateProgram();
		// Attach the Vertex and Fragment Shaders to the Shader Program
		glAttachShader(ID, vertexShader);
		glAttachShader(ID, fragmentShader);
		glAttachShader(ID, geometryShader);
		// Wrap-up/Link all the shaders together into the Shader Program
		glLinkProgram(ID);
		// Checks if Shaders linked succesfully
		compileErrors(ID, "PROGRAM");

		// Delete the now useless Vertex and Fragment Shader objects
		glDeleteShader(vertexShader);
		glDeleteShader(geometryShader);
		glDeleteShader(fragmentShader);
	}
	catch (const std::exception& e) {
		std::cerr << "[Error] - To Create Shader (" << vPath << ", " << gPath << ", " << fPath << "):\n" << e.what() << "\n\n";
	}
} 

// Activates the Shader Program
void Shader::use()
{
	glUseProgram(ID);
}

// Deletes the Shader Program
void Shader::Delete()
{
	glDeleteProgram(ID);
}

// utility uniform functions
// ------------------------------------------------------------------------
void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, glm::mat4 matrix) {
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setMat3(const std::string& name, glm::mat3 matrix) {
	glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setVec3(const std::string& name, float v1, float v2, float v3) {
	glUniform3f(glGetUniformLocation(ID, name.c_str()), v1, v2, v3);

}

void Shader::setVec3(const std::string& name, glm::vec3 vector) {
	glUniform3f(glGetUniformLocation(ID, name.c_str()), vector.x, vector.y, vector.z);
}

void Shader::setVec4(const std::string& name, float v1, float v2, float v3, float v4) {
	glUniform4f(glGetUniformLocation(ID, name.c_str()), v1, v2, v3, v4);

}


void Shader::setVec2(const std::string& name, float v1, float v2) {
	glUniform2f(glGetUniformLocation(ID, name.c_str()), v1, v2);

}

void Shader::setVec2(const std::string& name, glm::vec2 vector) {
	glUniform2f(glGetUniformLocation(ID, name.c_str()), vector.x, vector.y);
}

void Shader::setIntArray(const std::string& name, int* values, int count) {
	for (int i = 0; i < count; ++i) {
		std::string indexedName = name + "[" + std::to_string(i) + "]";
		glUniform1i(glGetUniformLocation(ID, indexedName.c_str()), values[i]);
	}
}

// Textures Arrays
void Shader::setTexture2D(const std::string& name, GLuint texID, GLenum unit) const {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, texID);
	glUniform1i(glGetUniformLocation(ID, name.c_str()), unit);
}

void Shader::setTexture2DArray(const std::string& name, GLuint texID, GLenum unit) const {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texID);
	glUniform1i(glGetUniformLocation(ID, name.c_str()), unit);
}

void Shader::setTextureCube(const std::string& name, GLuint texID, GLenum unit) const {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
	glUniform1i(glGetUniformLocation(ID, name.c_str()), unit);
}

void Shader::setTextureCubeArray(const std::string& name, GLuint texID, GLenum unit) const {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, texID);
	glUniform1i(glGetUniformLocation(ID, name.c_str()), unit);
} 


// Checks if the different Shaders have compiled properly
void Shader::compileErrors(unsigned int shader, const char* type)
{
	// Stores status of compilation
	GLint hasCompiled;
	// Character array to store error message in
	char infoLog[1024];

	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
			std::cout << std::endl << "name of shader : " + name << std::endl;
			// aqui você pode lançar exceção se quiser:
			// throw std::runtime_error(infoLog);
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
			std::cout << std::endl << "name of shader : " + name << std::endl;
			// aqui você pode lançar exceção se quiser:
			// throw std::runtime_error(infoLog);
		}
	}
}

 




//std::string injectDefines(const std::string& code, const std::vector<std::pair<std::string, int>>& defines) {
//	std::string result;
//	for (auto& d : defines) {
//		result += "#define " + d.first + " " + std::to_string(d.second) + "\n";
//	}
//	result += code;
//	return result;
//}

//std::string UpdateDefine(const std::string& shaderCode, const std::string& defineName, int newValue) {
//	std::stringstream ss(shaderCode);
//	std::string line;
//	std::string newCode;
//	bool found = false;
//
//	while (std::getline(ss, line)) {
//		if (line.find("#define " + defineName) == 0) {
//			line = "#define " + defineName + " " + std::to_string(newValue);
//			found = true;
//		}
//		newCode += line + "\n";
//	}
//
//	if (!found) {
//		// Se não achar, adiciona no início
//		newCode = "#define " + defineName + " " + std::to_string(newValue) + "\n" + newCode;
//	}
//
//	return newCode;
//}