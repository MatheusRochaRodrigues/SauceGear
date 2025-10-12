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

    static std::unique_ptr<Mesh> Generate(const ChunkBuffer& buff, glm::vec3 uOffset, GLuint computeProgram,  GLuint ssboSDF = 0 ) {
        const int   uDim        = sysv.get_voxelGrid();      // número de pontos (ex: 33)
        const float uVoxelSize  = sysv.get_voxelSize();
        const float uWorldScale = sysv.get_chunkSize();
        const size_t voxelCount = size_t(uDim) * uDim * uDim;   //arraySize
         
        std::cout << voxelCount << " d " << buff.density.size() << std::endl;
        assert(voxelCount == buff.density.size());

        // --- 5. Bind & set uniforms ---
        glUseProgram(computeProgram); 

        glUniform1i(glGetUniformLocation (computeProgram, "uDim"), uDim);
        glUniform1f(glGetUniformLocation (computeProgram, "uVoxelSize"), uVoxelSize);
        glUniform1f(glGetUniformLocation (computeProgram, "uWorldScale"), uWorldScale);
        glUniform3fv(glGetUniformLocation(computeProgram, "uOffset"), 1, glm::value_ptr(uOffset));

         
        // --- 1. SSBO: SDF input ---
        if(ssboSDF == 0) ssboSDF = CreateSSBO((sizeof(float) * voxelCount) + 1, buff.density.data(), GL_STATIC_DRAW);
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
        GLuint gx = (uDim + 7) / 8;
        GLuint gy = (uDim + 7) / 8;
        GLuint gz = (uDim + 7) / 8;

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

        // --- 9. Cleanup (opcional, ou mantenha se quiser reusar) ---
        glDeleteBuffers(1, &ssboSDF);
        glDeleteBuffers(1, &ssboPositions);
        glDeleteBuffers(1, &ssboNormals);
        glDeleteBuffers(1, &ssboIndices);
        glDeleteBuffers(1, &ssboStrideToIndex);
        glDeleteBuffers(1, &ssboCounters); 
         
        // --- Create Mesh ---
        auto mesh = std::make_unique<Mesh>();
        mesh->UploadFromRaw(positions, normals, indices);  //mesh->UploadFromRaw(positions, normals, indices); 
        return mesh;
    }
};
