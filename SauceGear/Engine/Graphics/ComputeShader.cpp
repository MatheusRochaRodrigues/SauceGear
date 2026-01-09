#include "ComputeShader.h"
#include <iostream>

ComputeShader::ComputeShader(const char* file, const std::vector<std::pair<std::string, int>>& defines, bool debug) {
    computeFile = file;
    name = file;

    try {
        std::string code = ProcessFileWithDefines((ShaderPathDefault + path + file).c_str(), defines, debug);
        const char* src = code.c_str();

        GLuint cShader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(cShader, 1, &src, nullptr);
        glCompileShader(cShader);
        compileErrors(cShader, "COMPUTE");

        ID = glCreateProgram();
        glAttachShader(ID, cShader);
        glLinkProgram(ID);
        compileErrors(ID, "PROGRAM");

        glDeleteShader(cShader);
    }
    catch (const std::exception& e) {
        std::cerr << "[ComputeShader Error] " << e.what() << "\n";
    }
}

void ComputeShader::ReloadWithDefines(const std::vector<std::pair<std::string, int>>& defines) {
    if (computeFile) {
        ComputeShader temp(computeFile, defines, true);
        ID = temp.ID;
    }
}

void ComputeShader::Dispatch(GLuint groupsX, GLuint groupsY, GLuint groupsZ, bool barrier) const {
    use();
    glDispatchCompute(groupsX, groupsY, groupsZ);
    if (barrier) glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}




void ComputeShader::setBuffer(const std::string& name, GLuint bufferID, GLuint bindingPoint) const {
    GLuint index = glGetProgramResourceIndex(ID, GL_SHADER_STORAGE_BLOCK, name.c_str());
    if (index != GL_INVALID_INDEX)
        glShaderStorageBlockBinding(ID, index, bindingPoint);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, bufferID);
}

void ComputeShader::setTexture2D(const std::string& name, GLuint texID, GLenum unit) const {
    glBindImageTexture(unit, texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}