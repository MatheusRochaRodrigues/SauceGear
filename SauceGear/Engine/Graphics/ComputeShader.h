#pragma once
#include "ShaderBase.h"
#include <vector>
#include <utility>

class ComputeShader : public ShaderBase {
public:
    const char* computeFile = nullptr;
    std::string path = "Compute/";

    ComputeShader() = default;
    ComputeShader(const char* file, const std::vector<std::pair<std::string, int>>& defines = {}, bool debug = false);

    void ReloadWithDefines(const std::vector<std::pair<std::string, int>>& defines);

    void Dispatch(GLuint groupsX, GLuint groupsY, GLuint groupsZ, bool barrier = true) const;
     
    void setTexture2D(const std::string& name, GLuint texID, GLenum unit) const;
    void setBuffer(const std::string& name, GLuint bufferID, GLuint bindingPoint) const;

    ~ComputeShader() { 
        //if (computeFile) free((void*)computeFile);
    }




    GLuint LoadComputeShader(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return 0;
        std::stringstream ss;
        ss << file.rdbuf();
        std::string src = ss.str();
        const char* csrc = src.c_str();

        GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shader, 1, &csrc, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::cerr << "Compute shader compile error:\n" << log << std::endl;
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, shader);
        glLinkProgram(program);
        glDeleteShader(shader);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char log[512];
            glGetProgramInfoLog(program, 512, nullptr, log);
            std::cerr << "Compute shader link error:\n" << log << std::endl;
            return 0;
        }

        return program;
    }
};
