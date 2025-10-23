#include "SurfaceNets.h"
#include "../Graphics/Mesh.h"
#include "SurfaceNetsGPUBuffer.h"
#include <glad/glad.h>
#include <vector>
#include <iostream>
#include <cassert>

#define sysv SysVoxel::getInstance()

class SurfaceNetsGPU {
public:

    static GLuint CreateSSBO(GLsizeiptr size, const void* data = nullptr, GLenum usage = GL_DYNAMIC_COPY) {
        GLuint id;
        glGenBuffers(1, &id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        return id;
    }

    // Cria ou realoca SSBO conforme necessário             util: (re)aloca um SSBO para 'newCount' elementos
    static void EnsureSSBO(GLuint& ssbo, size_t currentCount, size_t newCount, size_t elementSize, GLenum usage = GL_DYNAMIC_COPY) {
        size_t newSize = newCount * elementSize;
        size_t oldSize = currentCount * elementSize;
  
        if (ssbo == 0) {
            ssbo = CreateSSBO(newSize, nullptr, usage);  // criar novo buffer se ainda não existir 
            return;
        }
        
        if (newSize > oldSize) {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
            glBufferData(GL_SHADER_STORAGE_BUFFER, newSize, nullptr, usage);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }
    }
      
    static std::unique_ptr<Mesh> Generate(
        const ChunkBuffer& buff,
        glm::vec3 uOffset,
        GLuint computeProgram,
        SurfaceNetsGPUBuffer& gpuBuff,
        GLuint ssboSDF = 0 )
    {
        const int   DimCells  = sysv.get_cellGrid();      // ex: 32
        const int   DimVoxel  = sysv.get_voxelGrid();     // ex: 33
        const float VoxelSize = sysv.get_voxelSize();

        const size_t voxelCount = size_t(DimVoxel) * DimVoxel * DimVoxel;
        assert(voxelCount == buff.densityMap.size());

        // Só realocar se necessário
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

        glUseProgram(computeProgram);
        glUniform1i(glGetUniformLocation(computeProgram, "uDim"), DimVoxel);
        glUniform1f(glGetUniformLocation(computeProgram, "uVoxelSize"), VoxelSize);
        glUniform3fv(glGetUniformLocation(computeProgram, "uOffset"), 1, glm::value_ptr(uOffset));

        // --- SDF input ---
        //if (ssboSDF == 0) ssboSDF = CreateSSBO(voxelCount * sizeof(float), buff.densityMap.data(), GL_DYNAMIC_DRAW);    //GL_STATIC_DRAW
        //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboSDF);


        // --- Buffer SDF ---
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboSDF);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, voxelCount * sizeof(float), buff.densityMap.data());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gpuBuff.ssboSDF);
         



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


        // --- Reset stride table ---
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

        // --- Cria Mesh ---
        auto mesh = std::make_unique<Mesh>();
        mesh->UploadFromRaw(positions, normals, indices);
        return mesh;
    }



    // Functor de hash para tuple<int,int,int>
    struct TupleHash {
        size_t operator()(const std::tuple<int, int, int>& t) const noexcept {
            auto [x, y, z] = t;
            // Combina os 3 inteiros em um hash único
            size_t h1 = std::hash<int>()(x);
            size_t h2 = std::hash<int>()(y);
            size_t h3 = std::hash<int>()(z);
            return h1 ^ (h2 << 1) ^ (h3 << 2); // combinação simples e eficiente
        }
    };


    static void getDebug(uint32_t voxelCount, GLuint computeProgram, GLuint gx, GLuint gy, GLuint gz) {
        glUniform1i(glGetUniformLocation(computeProgram, "uPass"), 3);
        // === Buffers de debug ===
        const size_t maxDebugPoints = voxelCount * 64; // cantos + edges
        GLuint ssboDebugPos = CreateSSBO(sizeof(glm::vec4) * maxDebugPoints);
        GLuint ssboDebugCol = CreateSSBO(sizeof(glm::vec4) * maxDebugPoints);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssboDebugPos);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssboDebugCol);

        GLuint ssboDebugCounter = CreateSSBO(sizeof(GLuint)); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, ssboDebugCounter);
        GLuint zero = 0; glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &zero);

        glDispatchCompute(gx, gy, gz);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

        // === Ler debug data ===
        GLuint debugCount = 0;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDebugCounter);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &debugCount);

        if (debugCount > maxDebugPoints) debugCount = maxDebugPoints;   //n entendi

        std::vector<glm::vec4> dbgPositions(debugCount);
        std::vector<glm::vec4> dbgColors(debugCount);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDebugPos);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * debugCount, dbgPositions.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDebugCol);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * debugCount, dbgColors.data());

        for (size_t i = 0; i < dbgPositions.size(); ++i) {
            glm::vec3 pos = glm::vec3(dbgPositions[i]);
            glm::vec3 col = glm::vec3(dbgColors[i]);
            DebugRenderer::AddPoint(pos, col, 6.0f, DebugPointType::Square, true);
        }

        /*for (size_t i = 0; i + 1 < dbgPositions.size(); i += 2) {
            glm::vec3 a =   glm::vec3(dbgPositions[i]);
            glm::vec3 b =   glm::vec3(dbgPositions[i + 1]);
            glm::vec3 col = glm::vec3(dbgColors[i]);
            DebugRenderer::AddLine(a, b, col, true);
        }*/


        // função para transformar glm::vec3 em chave discreta
        auto posToKey = [](const glm::vec3& p) {
            return std::make_tuple(int(std::round(p.x)), int(std::round(p.y)), int(std::round(p.z)));
            };


        // criar set de pontos ocupados
        std::unordered_set<std::tuple<int, int, int>, TupleHash> occupied;

        // preencher com pontos do shader
        for (auto& p : dbgPositions) {
            occupied.insert(posToKey(glm::vec3(p)));
        }

        glm::vec3 numChunks = glm::vec3(sysv.get_voxelGrid());
        for (int cz = 0; cz < numChunks.z; ++cz) for (int cy = 0; cy < numChunks.y; ++cy) for (int cx = 0; cx < numChunks.x; ++cx)
        {
            glm::vec3 offset = glm::vec3(cx, cy, cz) * (float)sysv.get_voxelSize();
            if (occupied.find(posToKey(offset)) == occupied.end())
            {
                // ponto não existe, adicionar como preto
                DebugRenderer::AddPoint(offset, glm::vec3(0.0f), 6.0f, DebugPointType::Square, true);
            }
        }


        // === Opcional: deletar SSBOs depois se não for reaproveitar ===
        glDeleteBuffers(1, &ssboDebugPos);
        glDeleteBuffers(1, &ssboDebugCol);
        glDeleteBuffers(1, &ssboDebugCounter);
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
