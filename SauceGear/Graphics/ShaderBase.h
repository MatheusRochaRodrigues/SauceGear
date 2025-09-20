#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include "ShaderPreprocessor.h"

class ShaderBase {
public:
    GLuint ID = 0;
    std::string name;

    ShaderBase() = default;
    virtual ~ShaderBase() = default;

    virtual void use() const { glUseProgram(ID); }
    virtual void Delete() { glDeleteProgram(ID); }

    // ---- Uniform setters ----
    void setBool(const std::string& n, bool v) const { glUniform1i(glGetUniformLocation(ID, n.c_str()), (int)v); }
    void setInt(const std::string& n, int v) const { glUniform1i(glGetUniformLocation(ID, n.c_str()), v); }
    void setFloat(const std::string& n, float v) const { glUniform1f(glGetUniformLocation(ID, n.c_str()), v); }

    void setVec2(const std::string& n, float x, float y) { glUniform2f(glGetUniformLocation(ID, n.c_str()), x, y); }
    void setVec2(const std::string& n, glm::vec2 v) { glUniform2f(glGetUniformLocation(ID, n.c_str()), v.x, v.y); }

    void setVec3(const std::string& n, float x, float y, float z) { glUniform3f(glGetUniformLocation(ID, n.c_str()), x, y, z); }
    void setVec3(const std::string& n, glm::vec3 v) { glUniform3f(glGetUniformLocation(ID, n.c_str()), v.x, v.y, v.z); }

    void setVec4(const std::string& n, float x, float y, float z, float w) { glUniform4f(glGetUniformLocation(ID, n.c_str()), x, y, z, w); }
    void setVec4(const std::string& n, glm::vec4 v) { glUniform4f(glGetUniformLocation(ID, n.c_str()), v.x, v.y, v.z, v.w); }

    void setMat3(const std::string& n, glm::mat3 m) { glUniformMatrix3fv(glGetUniformLocation(ID, n.c_str()), 1, GL_FALSE, glm::value_ptr(m)); }
    void setMat4(const std::string& n, glm::mat4 m) { glUniformMatrix4fv(glGetUniformLocation(ID, n.c_str()), 1, GL_FALSE, glm::value_ptr(m)); }

    void setIntArray(const std::string& n, int* values, int count) {
        for (int i = 0; i < count; ++i) {
            std::string indexed = n + "[" + std::to_string(i) + "]";
            glUniform1i(glGetUniformLocation(ID, indexed.c_str()), values[i]);
        }
    }

    // ---- Textures ----
    void setTexture2D(const std::string& n, GLuint tex, GLenum unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(glGetUniformLocation(ID, n.c_str()), unit);
    }

    void setTexture2DArray(const std::string& n, GLuint tex, GLenum unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
        glUniform1i(glGetUniformLocation(ID, n.c_str()), unit);
    }

    void setTextureCube(const std::string& n, GLuint tex, GLenum unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
        glUniform1i(glGetUniformLocation(ID, n.c_str()), unit);
    }

    void setTextureCubeArray(const std::string& n, GLuint tex, GLenum unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, tex);
        glUniform1i(glGetUniformLocation(ID, n.c_str()), unit);
    }

protected:
    void compileErrors(GLuint shader, const char* type) {
        GLint success;
        char info[1024];
        if (std::string(type) != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, nullptr, info);
                std::cerr << "SHADER_COMPILATION_ERROR (" << type << "): " << info << "\nname: " << name << "\n";
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, nullptr, info);
                std::cerr << "SHADER_LINKING_ERROR (" << type << "): " << info << "\nname: " << name << "\n";
            }
        }
    }

    std::string ShaderPathDefault = "Resources/Shaders/";

    std::string ProcessFileWithDefines(const char* file, const std::vector<std::pair<std::string, int>>& defines, bool debug) {
        return ShaderPreprocessor::ProcessFile(file, defines, debug);
    }
};
