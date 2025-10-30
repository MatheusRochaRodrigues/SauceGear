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
        bool readySSBO_SDF = true )
    {
        const int   DimCells  = sysv.get_cellGrid();      // ex: 32
        const int   DimVoxel  = sysv.get_voxelGrid();     // ex: 33
        const float VoxelSize = sysv.get_voxelSize();

        const size_t voxelCount = size_t(DimVoxel) * DimVoxel * DimVoxel;
        assert(voxelCount == buff.densityMap.size());

        // Ensure capacity (GL thread). If not enough, grow.
        //if (gpuBuff.allocatedVoxels < voxelCount) gpuBuff.ensureCapacity(voxelCount); 


        // Só realocar se necessário
        /*
        if (gpuBuff.allocatedVoxels < voxelCount) {
            gpuBuff.allocatedVoxels = voxelCount;

            //Size Info      maxVertices == voxelCount          /           maxIndices = voxelCount * 6u
            EnsureSSBO(gpuBuff.ssboSDF,             0, voxelCount,      sizeof(float),      GL_DYNAMIC_DRAW);
            EnsureSSBO(gpuBuff.ssboPositions,       0, voxelCount,      sizeof(glm::vec4),  GL_DYNAMIC_COPY);
            EnsureSSBO(gpuBuff.ssboNormals,         0, voxelCount,      sizeof(glm::vec4),  GL_DYNAMIC_COPY);
            EnsureSSBO(gpuBuff.ssboIndices,         0, voxelCount * 6u, sizeof(uint32_t),   GL_DYNAMIC_COPY);
            EnsureSSBO(gpuBuff.ssboStrideToIndex,   0, voxelCount,      sizeof(uint32_t),   GL_DYNAMIC_COPY);
            EnsureSSBO(gpuBuff.ssboCounters,        0, 2,               sizeof(uint32_t),   GL_DYNAMIC_COPY);
        }
        */

        glUseProgram(computeProgram);
        glUniform1i(glGetUniformLocation(computeProgram, "uDim"), DimVoxel);
        glUniform1f(glGetUniformLocation(computeProgram, "uVoxelSize"), VoxelSize);
        glUniform3fv(glGetUniformLocation(computeProgram, "uOffset"), 1, glm::value_ptr(uOffset));

        //glUniform1i(glGetUniformLocation(computeProgram, "uEvalMaxPlane"), (int)0);

        // --- SDF input ---
        //if (ssboSDF == 0) ssboSDF = CreateSSBO(voxelCount * sizeof(float), buff.densityMap.data(), GL_DYNAMIC_DRAW);    //GL_STATIC_DRAW
        //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboSDF); 

        // --- Buffer SDF ---   
        // Bind or upload SDF:
        if (readySSBO_SDF) {
            // Bind external SDF already prepared by caller
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gpuBuff.ssboSDF);
        } else {
            // upload buff.densityMap into gpuBuff.ssboSDF
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboSDF);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, voxelCount * sizeof(float), buff.densityMap.data());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gpuBuff.ssboSDF);
        }


        // --- Bind --- Outputs ---
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gpuBuff.ssboPositions);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, gpuBuff.ssboNormals);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, gpuBuff.ssboIndices);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, gpuBuff.ssboStrideToIndex);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, gpuBuff.ssboCounters);
          

        // --- Reset counters ---
        //Bind
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboCounters); 
        //Data Operation
        uint32_t zero[2] = { 0, 0 };
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t) * 2, zero);   //sizeof(uint32_t) * 2 == sizeof(zero)
        //Unbind
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


        // --- Reset stride table ---                Reset stride_to_index
        std::vector<uint32_t> initStride(voxelCount, 0xFFFFFFFFu);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboStrideToIndex);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, voxelCount * sizeof(uint32_t), initStride.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


        // --- Dispatch compute ---
        GLuint gx = (DimCells + 7) / 8;
        GLuint gy = (DimCells + 7) / 8;
        GLuint gz = (DimCells + 7) / 8;


        // PASS 1 - vertices
        glUniform1i(glGetUniformLocation(computeProgram, "uPass"), 1);
        glDispatchCompute(gx, gy, gz);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // PASS 2 - indices
        glUniform1i(glGetUniformLocation(computeProgram, "uPass"), 2);
        glDispatchCompute(gx, gy, gz);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


        // --- Ler resultados ---
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboCounters);
        uint32_t counts[2];
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(counts), counts);
        uint32_t vertexCount    = counts[0];
        uint32_t indexCount     = counts[1];

        std::vector<glm::vec4> positions(vertexCount);
        std::vector<glm::vec4> normals  (vertexCount);
        std::vector<uint32_t>  indices  (indexCount);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboPositions);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, vertexCount * sizeof(glm::vec4),    positions.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboNormals);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, vertexCount * sizeof(glm::vec4),    normals.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboIndices);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, indexCount * sizeof(uint32_t),      indices.data());
         
        // 2️⃣ desenha as linhas da grade
        //DebugVoxels::drawVoxelGrid(uOffset);
        //DebugVoxels::getDebug2(voxelCount, computeProgram, gx, gy, gz, gpuBuff, uOffset);
        //DebugVoxels::getDebugCentroid(positions);
         
        //getDebug(voxelCount, computeProgram, gx, gy, gz); 


        if (positions.empty()) std::cout << "esta vazia positions" << std::endl;
        else std::cout << "nao esta vazia positions" << std::endl;

        if (indices.empty()) std::cout << "esta vazia indices" << std::endl;
        else std::cout << "nao esta vazia indices" << std::endl;

        if (positions.empty() && indices.empty()) return nullptr;


        // --- Cria Mesh ---
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
        glUniform1f(glGetUniformLocation(computeProgram, "uVoxelSize"), sysv.get_voxelSize());
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
