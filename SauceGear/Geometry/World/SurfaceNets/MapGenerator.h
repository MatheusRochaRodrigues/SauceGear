#pragma once 
#include "../Graphics/ComputeShader.h" 
#include "../Geometry/World/SurfaceNets/SurfaceNets.h"   

class GPUMapGenerator {
public:
    ComputeShader compute;
    GLuint ssbo = 0; size_t size_ssbo = 0;

    GPUMapGenerator(const char* shaderFile) : compute(shaderFile) {  Init(sysv.get_voxelGrid()); }

    // inicializa o SSBO
    void Init(int dim) {
        // cria SSBO para SDF
        if (ssbo == 0) glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, dim * dim * dim * sizeof(float), nullptr, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); 
        size_ssbo = size_t(dim) * dim * dim;
    }

    void Generate(glm::vec3 worldOffset, ChunkBuffer& buff) {
        int dim = sysv.get_voxelGrid();
        size_t dim3 = size_t(dim) * dim * dim;

        if (dim3 != size_ssbo) Init(dim);

        // redimensiona CPU buffer
        if (buff.densityMap.size() != dim3) buff.densityMap.resize(dim3, 1.0f);

        // set uniforms
        glUseProgram(compute.ID);
        glUniform1i(glGetUniformLocation(compute.ID, "uDim"), dim);
        glUniform1f(glGetUniformLocation(compute.ID, "uVoxelSize"), sysv.get_voxelSize());
        glUniform1f(glGetUniformLocation(compute.ID, "uIsoLevel"), sysv.isoLevel);
        glUniform3f(glGetUniformLocation(compute.ID, "uOffset"), worldOffset.x, worldOffset.y, worldOffset.z);

        // despacha compute shader
        GLuint gx = (dim + 7) / 8;
        GLuint gy = (dim + 7) / 8;
        GLuint gz = (dim + 7) / 8;
        compute.Dispatch(gx, gy, gz, true);

        // lê de volta para CPU
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, dim * dim * dim * sizeof(float), buff.densityMap.data());
    }

    ~GPUMapGenerator() {
        if (ssbo) glDeleteBuffers(1, &ssbo);
    }
};