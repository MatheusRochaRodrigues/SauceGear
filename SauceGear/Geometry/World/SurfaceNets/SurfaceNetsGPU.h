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

    void EnsureSSBO(GLuint& ssbo, size_t& currentSize, size_t newSize, GLenum usage = GL_DYNAMIC_COPY) {
        if (ssbo == 0) { 
            ssbo = CreateSSBO(newSize, nullptr, usage);  // criar novo buffer se ainda não existir
            currentSize = newSize;
        } else {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo); 
            // verificar se precisa expandir
            if (newSize > currentSize) {
                glBufferData(GL_SHADER_STORAGE_BUFFER, newSize, nullptr, usage);
                currentSize = newSize;
            } 
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }
    } 

    static GLuint CreateSSBO(GLsizeiptr size, const void* data = nullptr, GLenum usage = GL_DYNAMIC_COPY) {
        GLuint id;
        glGenBuffers(1, &id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        return id;
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

    static std::unique_ptr<Mesh> Generate(const ChunkBuffer& buff, glm::vec3 uOffset, GLuint computeProgram,  GLuint ssboSDF = 0 ) {
        const int   DimCells   = sysv.get_cellGrid();       // número de celulas (ex: 32)
        const float VoxelSize  = sysv.get_voxelSize();      
        //const float WorldScale = sysv.get_chunkSize();

        const int DimVoxel = sysv.get_voxelGrid();          // número de pontos  (ex: 33)
        const size_t voxelCount = size_t(DimVoxel) * DimVoxel * DimVoxel;   //arraySize 
        assert(voxelCount == buff.densityMap.size());

        // --- 5. Bind & set uniforms ---
        glUseProgram(computeProgram); 

        glUniform1i(glGetUniformLocation (computeProgram, "uDim"), DimVoxel);
        glUniform1f(glGetUniformLocation (computeProgram, "uVoxelSize"), VoxelSize); 
        glUniform3fv(glGetUniformLocation(computeProgram, "uOffset"), 1, glm::value_ptr(uOffset));

         
        // --- 1. SSBO: SDF input ---
        if(ssboSDF == 0) ssboSDF = CreateSSBO((sizeof(float) * voxelCount), buff.densityMap.data(), GL_STATIC_DRAW);    //+1
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboSDF);

        // --- 2. SSBO: Outputs ---
        const size_t maxVertices = voxelCount;
        const size_t maxIndices = voxelCount * 6u;

        GLuint ssboPositions = CreateSSBO(sizeof(glm::vec4) * maxVertices);
        GLuint ssboNormals   = CreateSSBO(sizeof(glm::vec4) * maxVertices);
        GLuint ssboIndices   = CreateSSBO(sizeof(uint32_t)  * maxIndices);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboPositions);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboNormals);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboIndices);

        // --- 3. SSBO: stride_to_index ---
        GLuint ssboStrideToIndex = CreateSSBO(sizeof(uint32_t) * voxelCount, nullptr, GL_DYNAMIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboStrideToIndex);
        std::vector<uint32_t> initStride(voxelCount, NULL_VERTEX);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, voxelCount * sizeof(uint32_t), initStride.data());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboStrideToIndex);

        // --- 4. SSBO: Counters (vertexCount, indexCount) ---
        GLuint ssboCounters = CreateSSBO(sizeof(uint32_t) * 2);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCounters);
        uint32_t zero[2] = { 0, 0 };
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t) * 2, zero);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssboCounters); 

        // --- 6. Dispatch compute ---
        GLuint gx = (DimCells + 7) / 8;
        GLuint gy = (DimCells + 7) / 8;
        GLuint gz = (DimCells + 7) / 8;

        // ------------------------
        // --- PASS 1: vertices ---
        // ------------------------
        glUniform1i(glGetUniformLocation(computeProgram, "uPass"), 1);
        glDispatchCompute(gx, gy, gz);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // ------------------------
        // --- PASS 2: indices  ---
        // ------------------------
        glUniform1i(glGetUniformLocation(computeProgram, "uPass"), 2);
        glDispatchCompute(gx, gy, gz);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
         
        // --- 7. Ler counters ---
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCounters);
        uint32_t counts[2];
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(counts), counts);
        uint32_t vertexCount = counts[0];
        uint32_t indexCount  = counts[1];
        std::cout << "SurfaceNetsGPU -> vertices: " << vertexCount << "  indices: " << indexCount << "\n";

        // --- 8. Ler dados resultantes ---
        std::vector<glm::vec4>  positions;  positions.resize(vertexCount);
        std::vector<glm::vec4>  normals;    normals.resize(vertexCount);       // not normalized (normalize on GPU if desired)         
        std::vector<uint32_t>   indices;    indices.resize(indexCount);
         
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPositions);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * vertexCount, positions.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNormals);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * vertexCount, normals.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndices);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t)  * indexCount , indices.data());


        
        //Pass Extra - 3 Debug
        getDebug(voxelCount,computeProgram, gx, gy ,gz);



        // --- 9. Cleanup (opcional, ou mantenha se quiser reusar) ---
        glDeleteBuffers(1, &ssboSDF);
        glDeleteBuffers(1, &ssboPositions);
        glDeleteBuffers(1, &ssboNormals);
        glDeleteBuffers(1, &ssboIndices);
        glDeleteBuffers(1, &ssboStrideToIndex);
        glDeleteBuffers(1, &ssboCounters); 

        // verifica se algum índice está fora do alcance
        //CheckIndices(positions, indices);

        // --- Create Mesh ---
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


};
