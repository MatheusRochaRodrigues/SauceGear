#pragma once
#include "SurfaceNets.h"
#include "../Graphics/Mesh.h"
#include "SurfaceNetsGPUBuffer.h"
#include <glad/glad.h>
#include <vector>
#include <iostream>
#include <cassert>

#include "DebugVoxels.h"


//#define sysv SysVoxel::getInstance()

class SurfaceNetsGPU {
public: 
    static std::unique_ptr<Mesh> Generate(
        const ChunkBuffer& buff,
        glm::vec3 uOffset,
        GLuint computeProgram,
        SurfaceNetsGPUBuffer& gpuBuff,
        bool readySSBO_SDF = true,
        int GridVoxelPerAxis = 33,   // LOD-aware
        float voxelSize = 1.0f
    ) {
        const size_t voxelCount = size_t(GridVoxelPerAxis) * GridVoxelPerAxis * GridVoxelPerAxis;

        std::cout << " wqe " << buff.densityMap.size() << std::endl;
        std::cout << " wqe2 " << voxelCount << std::endl;

        assert(buff.densityMap.size() == voxelCount && "Density map size mismatch!");

        glUseProgram(computeProgram);

        // Passa uniforms para shader
        glUniform1i(glGetUniformLocation(computeProgram, "uDim"), GridVoxelPerAxis);
        glUniform1f(glGetUniformLocation(computeProgram, "uVoxelSize"), voxelSize);
        glUniform3fv(glGetUniformLocation(computeProgram, "uOffset"), 1, glm::value_ptr(uOffset));

        // Bind SDF
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gpuBuff.ssboSDF);
        if (!readySSBO_SDF) {
            std::cout << " ck " << buff.densityMap[100] << std::endl;
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboSDF);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, voxelCount * sizeof(float), buff.densityMap.data());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gpuBuff.ssboSDF);
        }

        // Bind outputs
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gpuBuff.ssboPositions);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, gpuBuff.ssboNormals);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, gpuBuff.ssboIndices);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, gpuBuff.ssboStrideToIndex);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, gpuBuff.ssboCounters);

        // Reset counters e stride table
        uint32_t zero[2] = { 0,0 };
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboCounters);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(zero), zero);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        std::vector<uint32_t> initStride(voxelCount, 0xFFFFFFFFu);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboStrideToIndex);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, voxelCount * sizeof(uint32_t), initStride.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // Dispatch compute: vertices e indices
        GLuint gx = (GridVoxelPerAxis + 7) / 8;
        GLuint gy = gx;
        GLuint gz = gx;

        // Pass 1: generate vertices
        glUniform1i(glGetUniformLocation(computeProgram, "uPass"), 1);
        glDispatchCompute(gx, gy, gz);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Pass 2: generate indices
        glUniform1i(glGetUniformLocation(computeProgram, "uPass"), 2);
        glDispatchCompute(gx, gy, gz);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Ler contadores
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboCounters);
        uint32_t counts[2];
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(counts), counts);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        uint32_t vertexCount = counts[0];
        uint32_t indexCount = counts[1];

        //if (vertexCount == 0 || indexCount == 0) return nullptr;

        // Ler buffers
        std::vector<glm::vec4> positions(vertexCount);
        std::vector<glm::vec4> normals(vertexCount);
        std::vector<uint32_t>  indices(indexCount);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboPositions);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, vertexCount * sizeof(glm::vec4), positions.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboNormals);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, vertexCount * sizeof(glm::vec4), normals.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboIndices);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, indexCount * sizeof(uint32_t), indices.data());




        // 2️⃣ desenha as linhas da grade
        //DebugVoxels::drawVoxelGrid(uOffset);
        //DebugVoxels::getDebug2(voxelCount, computeProgram, gx, gy, gz, gpuBuff, uOffset);
        //DebugVoxels::getDebugCentroid(positions);

        //getDebug(voxelCount, computeProgram, gx, gy, gz); 


        /*if (positions.empty()) std::cout << "esta vazia positions" << std::endl;
        else std::cout << "nao esta vazia positions" << std::endl;

        if (indices.empty()) std::cout << "esta vazia indices" << std::endl;
        else std::cout << "nao esta vazia indices" << std::endl;*/

        if (vertexCount == 0 || indexCount == 0) return nullptr;


        std::cout << "possui indices" << std::endl;


        // Cria Mesh
        auto mesh = std::make_unique<Mesh>();
        mesh->UploadFromRaw(positions, normals, indices);

        return mesh;
    }



    // Dispatch compute only (no readback). Must bind SSBOs appropriately and not readback.
    static void DispatchOnly(const ChunkBuffer& buff, glm::vec3 uOffset, GLuint computeProgram, SurfaceNetsGPUBuffer& gpuBuff, bool sdfAlreadyOnGPU = true, GLuint externalSDF = 0) {
        const int   DimCells = sysv.get_cellGrid();
        const int   DimVoxel = sysv.get_voxelGrid();
        const size_t voxelCount = size_t(DimVoxel) * DimVoxel * DimVoxel;

        // assume gpuBuff.ensureCapacity(voxelCount) was already called

        glUseProgram(computeProgram);
        glUniform1i(glGetUniformLocation(computeProgram, "uDim"), DimVoxel);
        //glUniform1f(glGetUniformLocation(computeProgram, "uVoxelSize"), sysv.get_voxelSize( /*tst*/ 0));
        glUniform3fv(glGetUniformLocation(computeProgram, "uOffset"), 1, glm::value_ptr(uOffset));

        // bind SDF
        if (sdfAlreadyOnGPU && externalSDF != 0) {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, externalSDF);
        }
        else {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboSDF);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, voxelCount * sizeof(float), buff.densityMap.data());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gpuBuff.ssboSDF);
        }

        // Bind outputs
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gpuBuff.ssboPositions);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, gpuBuff.ssboNormals);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, gpuBuff.ssboIndices);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, gpuBuff.ssboStrideToIndex);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, gpuBuff.ssboCounters);

        // reset counters & stride
        uint32_t zero[2] = { 0,0 };
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboCounters);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(zero), zero);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        std::vector<uint32_t> initStride(voxelCount, 0xFFFFFFFFu);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboStrideToIndex);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, voxelCount * sizeof(uint32_t), initStride.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        GLuint gx = (DimCells + 7) / 8;
        GLuint gy = (DimCells + 7) / 8;
        GLuint gz = (DimCells + 7) / 8;

        // PASS 1
        glUniform1i(glGetUniformLocation(computeProgram, "uPass"), 1);
        glDispatchCompute(gx, gy, gz);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // PASS 2
        glUniform1i(glGetUniformLocation(computeProgram, "uPass"), 2);
        glDispatchCompute(gx, gy, gz);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Don't read back here.
    }

    // Readback mesh from gpuBuff (call on GL thread after fence signaled).
    static std::unique_ptr<Mesh> ReadbackMeshFromBuffer(SurfaceNetsGPUBuffer& gpuBuff, GLuint computeProgram) {
        const int DimVoxel = sysv.get_voxelGrid();
        const size_t voxelCount = size_t(DimVoxel) * DimVoxel * DimVoxel;

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboCounters);
        uint32_t counts[2];
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(counts), counts);
        uint32_t vertexCount = counts[0];
        uint32_t indexCount = counts[1];

        std::vector<glm::vec4> positions(vertexCount);
        std::vector<glm::vec4> normals(vertexCount);
        std::vector<uint32_t> indices(indexCount);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboPositions);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, vertexCount * sizeof(glm::vec4), positions.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboNormals);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, vertexCount * sizeof(glm::vec4), normals.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboIndices);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, indexCount * sizeof(uint32_t), indices.data());

        auto mesh = std::make_unique<Mesh>();
        mesh->UploadFromRaw(positions, normals, indices);
        return mesh;
    } 


    static void CheckIndices(const std::vector<glm::vec4>& positions, const std::vector<uint32_t>& indices) {
        size_t numVertices = positions.size();
        bool hasError = false;

        for (size_t i = 0; i < indices.size(); ++i) {
            uint32_t idx = indices[i];
            if (idx >= numVertices) {
                std::cerr << "ERRO: índice fora do alcance! "
                    << "indices[" << i << "] = " << idx
                    << ", mas numVertices = " << numVertices << std::endl;
                hasError = true;
            }
        }

        if (!hasError) {
            std::cout << "Todos os índices estão dentro do alcance do vetor de vértices." << std::endl;
        }
    }

};
