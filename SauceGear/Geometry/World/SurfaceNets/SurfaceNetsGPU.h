#include "SurfaceNets.h"
#include "../Graphics/Mesh.h"
#include <glad/glad.h>
#include <vector>
#include <iostream>
#include <cassert>

#define sysv SysVoxel::getInstance()

class SurfaceNetsGPU {
public:
    struct SurfaceNetsGPUBuffer {
        //GLuint ssboSDF = 0;
        GLuint ssboPositions = 0;
        GLuint ssboNormals = 0;
        GLuint ssboIndices = 0;
        GLuint ssboStrideToIndex = 0;
        GLuint ssboCounters = 0;
        size_t allocatedVoxels = 0; // para saber se precisa realocar

        // Tamanhos atualmente alocados (em bytes)
        //size_t sizeSDF = 0;
        size_t sizePositions = 0;
        size_t sizeNormals = 0;
        size_t sizeIndices = 0;
        size_t sizeStrideToIndex = 0;
        size_t sizeCounters = 0;
    };  

    static GLuint CreateSSBO(GLsizeiptr size, const void* data = nullptr, GLenum usage = GL_DYNAMIC_COPY) {
        GLuint id;
        glGenBuffers(1, &id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        return id;
    }

    void EnsureSSBO(GLuint& ssbo, size_t& currentSize, size_t newSize, GLenum usage = GL_DYNAMIC_COPY) {
        if (ssbo == 0) {
            ssbo = CreateSSBO(newSize, nullptr, usage);  // criar novo buffer se ainda não existir
            currentSize = newSize;
            return;
        }

        if (newSize > currentSize) {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
            glBufferData(GL_SHADER_STORAGE_BUFFER, newSize, nullptr, usage);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            currentSize = newSize;
        }
    }
      
    static std::unique_ptr<Mesh> Generate(const ChunkBuffer& buff,
        glm::vec3 uOffset,
        GLuint computeProgram,
        SurfaceNetsGPUBuffer& gpuBuf,
        GLuint ssboSDF = 0)
    {
        const int DimCells = sysv.get_cellGrid();      // ex: 32
        const int DimVoxel = sysv.get_voxelGrid();     // ex: 33
        const float VoxelSize = sysv.get_voxelSize();
        const size_t voxelCount = size_t(DimVoxel) * DimVoxel * DimVoxel;
        assert(voxelCount == buff.densityMap.size());

        glUseProgram(computeProgram);
        glUniform1i(glGetUniformLocation(computeProgram, "uDim"), DimVoxel);
        glUniform1f(glGetUniformLocation(computeProgram, "uVoxelSize"), VoxelSize);
        glUniform3fv(glGetUniformLocation(computeProgram, "uOffset"), 1, glm::value_ptr(uOffset));

        // --- SDF input ---
        if (ssboSDF == 0)
            ssboSDF = CreateSSBO(sizeof(float) * voxelCount, buff.densityMap.data(), GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboSDF);

        // --- Calcula tamanhos ---
        const size_t maxVertices = voxelCount;
        const size_t maxIndices = voxelCount * 6u;

        // --- Garantir buffers ---
        EnsureSSBO(gpuBuf.ssboPositions, gpuBuf.sizePositions, sizeof(glm::vec4) * maxVertices);
        EnsureSSBO(gpuBuf.ssboNormals, gpuBuf.sizeNormals, sizeof(glm::vec4) * maxVertices);
        EnsureSSBO(gpuBuf.ssboIndices, gpuBuf.sizeIndices, sizeof(uint32_t) * maxIndices);
        EnsureSSBO(gpuBuf.ssboStrideToIndex, gpuBuf.sizeStrideToIndex, sizeof(uint32_t) * voxelCount);
        EnsureSSBO(gpuBuf.ssboCounters, gpuBuf.sizeCounters, sizeof(uint32_t) * 2);

        gpuBuf.allocatedVoxels = voxelCount;

        // --- Bind ---
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gpuBuf.ssboPositions);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, gpuBuf.ssboNormals);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, gpuBuf.ssboIndices);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, gpuBuf.ssboStrideToIndex);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, gpuBuf.ssboCounters);

        // --- Reset counters ---
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuf.ssboCounters);
        uint32_t zero[2] = { 0, 0 };
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t) * 2, zero);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // --- Reset stride table ---
        std::vector<uint32_t> initStride(voxelCount, 0xFFFFFFFFu);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuf.ssboStrideToIndex);
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
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuf.ssboCounters);
        uint32_t counts[2];
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(counts), counts);
        uint32_t vertexCount = counts[0];
        uint32_t indexCount = counts[1];

        std::vector<glm::vec4> positions(vertexCount);
        std::vector<glm::vec4> normals(vertexCount);
        std::vector<uint32_t>  indices(indexCount);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuf.ssboPositions);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * vertexCount, positions.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuf.ssboNormals);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * vertexCount, normals.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuf.ssboIndices);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t) * indexCount, indices.data());

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
