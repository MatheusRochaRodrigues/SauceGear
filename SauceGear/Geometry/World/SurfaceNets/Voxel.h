#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../Graphics/ComputeShader.h" 
#include "../Geometry/World/SurfaceNets/SurfaceNetsGPU.h"   


class GPUMapGenerator {
public:
    ComputeShader compute;
    GLuint ssbo = 0;
    size_t size_ssbo = 0;

    GPUMapGenerator(const char* shaderFile) : compute(shaderFile) {}

    // inicializa o SSBO
    void Init(int dim) {
        // cria SSBO para SDF
        if (ssbo == 0) glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, dim * dim * dim * sizeof(float), nullptr, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void Generate(int dim, float voxelSize, VoxelGrid& out, SurfaceNetsParams& params, glm::vec3 uOffset) {
        voxelSize = params.worldSize / float(dim); 

        if (dim != size_ssbo) {
            size_ssbo = dim;
            out.sx = out.sy = out.sz = dim;
            out.density.resize(size_t(dim) * dim * dim, 0.0f);
        }

        // set uniforms
        glUseProgram(compute.ID);
        glUniform1i(glGetUniformLocation(compute.ID, "uDim"), dim);
        glUniform1f(glGetUniformLocation(compute.ID, "uVoxelSize"), voxelSize);
        glUniform1f(glGetUniformLocation(compute.ID, "uIsoLevel"), params.isoLevel);
        glUniform3f(glGetUniformLocation(compute.ID, "uOffset"), uOffset.x, uOffset.y, uOffset.z);

        // despacha compute shader
        GLuint gx = (dim + 7) / 8;
        GLuint gy = (dim + 7) / 8;
        GLuint gz = (dim + 7) / 8;
        compute.Dispatch(gx, gy, gz, true);

        // lê de volta para CPU
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, dim * dim * dim * sizeof(float), out.density.data());
    }

    ~GPUMapGenerator() {
        if (ssbo) glDeleteBuffers(1, &ssbo);
    }
};
 
class voxelSystem {
public:
    GPUMapGenerator* generator;
    SurfaceNetsParams* params;
    SurfaceNetsBuffer* buffer;
    VoxelGrid grid;

    voxelSystem() {
        std::cout << "o - 1" << std::endl;
        generator = new GPUMapGenerator("Map/VoxelMap.comp");
        std::cout << "o - 2" << std::endl;
        params = new SurfaceNetsParams(32, 1);
        std::cout << "o - 3" << std::endl;
        buffer = new SurfaceNetsBuffer();
        std::cout << "o - 4" << std::endl;
        computeShader = new ComputeShader("SurfaceNet/SurfaceNetsGPU.comp");
        std::cout << "o - 5" << std::endl;

        params->cellDimension += params->cellDimension + 1;

        generator->Init(params->cellDimension);         //VoxelGrid grid(params->cellDimension + 1, params->cellDimension + 1, params->cellDimension + 1);

        std::cout << "o - 6" << std::endl;
    }

    std::vector<Mesh*> gnrtChunk() {
        glm::vec3 numChunks = params->numChunks; 

        std::vector<Mesh*> meshes;
        meshes.resize(int(numChunks.x * numChunks.y * numChunks.z)); // pré-aloca espaço suficien

        int index = 0;
        for (int cz = 0; cz < numChunks.z; ++cz) for (int cy = 0; cy < numChunks.y; ++cy) for (int cx = 0; cx < numChunks.x; ++cx)
        {
            std::cout << "p 1" << std::endl;
            glm::vec3 offset = glm::vec3(cx, cy, cz) * (params->cellDimension * params->voxelSize * params->worldSize);

            std::cout << "p 2" << std::endl;
             //CUIDADO BUSQUE ATUALIZAR TODAS SUAS ESTRUTURAS POR SI SO SEMPRE QUE A DIMENSAO FOR ALTERADA
            generator->Generate(params->cellDimension, params->voxelSize, grid, *params, offset); // dim=64, voxelSize=1.0
            //grid.density = GeneratorMap::GenerateSphereSDF(params->cellDimension + 1, 10);
            // Agora grid.density está preenchido e pronto para o SurfaceNetsGPU 
            std::cout << "p 3" << std::endl;
            Mesh* mesh = SurfaceNetsGPU::Generate(grid, *params, *buffer, computeShader->ID, offset, 0);
            std::cout << "p 4" << std::endl;

            meshes[index++] = mesh; // insere diretamente na posição
        }
        std::cout << "p 5" << std::endl;

        return meshes; // retorna o vetor já totalmente preenchido
    }

private:
    ComputeShader* computeShader;
};
