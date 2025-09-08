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
        if (computeFile) free((void*)computeFile);
    }
};
