#include "Shader.h"
#include <iostream> 
 
Shader::Shader(string vertex, string fragment, const std::vector<std::pair<std::string, int>>& defines, bool debug) {
    vertexFile = vertex;
    fragmentFile = fragment;
    name = vertex;

    //path increment
    this->vertexFile   = ShaderPathDefault + path + vertexFile;
    this->fragmentFile = ShaderPathDefault + path + fragmentFile;
     
    try {
        std::string vCode = ProcessFileWithDefines(vertexFile.c_str(), defines, debug);
        std::string fCode = ProcessFileWithDefines(fragmentFile.c_str(), defines, debug);
        const char* vSrc = vCode.c_str();
        const char* fSrc = fCode.c_str();

        GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vShader, 1, &vSrc, nullptr);
        glCompileShader(vShader);
        compileErrors(vShader, "VERTEX");

        GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fShader, 1, &fSrc, nullptr);
        glCompileShader(fShader);
        compileErrors(fShader, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vShader);
        glAttachShader(ID, fShader);
        glLinkProgram(ID);
        compileErrors(ID, "PROGRAM");

        glDeleteShader(vShader);
        glDeleteShader(fShader);
    }
    catch (const std::exception& e) {
        std::cerr << "[Shader Error] " << e.what() << "\n";
    }
}

Shader::Shader(string vertex, string geometry, string fragment, const std::vector<std::pair<std::string, int>>& defines, bool debug) {
    vertexFile = vertex;
    geometryFile = geometry;
    fragmentFile = fragment;
    name = vertex;

    //path increment
    this->vertexFile   = ShaderPathDefault + path + vertexFile;
    this->geometryFile = ShaderPathDefault + path + geometryFile;
    this->fragmentFile = ShaderPathDefault + path + fragmentFile;

    try {
        std::string vCode = ProcessFileWithDefines(vertexFile.c_str(), defines, debug);
        std::string gCode = ProcessFileWithDefines(geometryFile.c_str(), defines, debug);
        std::string fCode = ProcessFileWithDefines(fragmentFile.c_str(), defines, debug);

        const char* vSrc = vCode.c_str();
        const char* gSrc = gCode.c_str();
        const char* fSrc = fCode.c_str();

        GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vShader, 1, &vSrc, nullptr);
        glCompileShader(vShader);
        compileErrors(vShader, "VERTEX");

        GLuint gShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gShader, 1, &gSrc, nullptr);
        glCompileShader(gShader);
        compileErrors(gShader, "GEOMETRY");

        GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fShader, 1, &fSrc, nullptr);
        glCompileShader(fShader);
        compileErrors(fShader, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vShader);
        glAttachShader(ID, gShader);
        glAttachShader(ID, fShader);
        glLinkProgram(ID);
        compileErrors(ID, "PROGRAM");

        glDeleteShader(vShader);
        glDeleteShader(gShader);
        glDeleteShader(fShader);
    }
    catch (const std::exception& e) {
        std::cerr << "[Shader Error] " << e.what() << "\n";
    }
}

void Shader::ReloadWithDefines(const std::vector<std::pair<std::string, int>>& defines) {
    if (!vertexFile.empty() && !fragmentFile.empty()) { 
        if (!geometryFile.empty()) {
            Shader temp( vertexFile.c_str(), geometryFile.c_str(), fragmentFile.c_str(), defines, true );
            ID = temp.ID;
        } else {
            Shader temp( vertexFile.c_str(), fragmentFile.c_str(), defines, true ); 
            ID = temp.ID;
        }

    }

}




//Shader::Shader(const char* vertex, const char* geometry, const char* fragment, const std::vector<std::pair<std::string, int>>& defines, bool debug) {
