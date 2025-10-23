#pragma once
#include "../Graphics/Mesh.h"
#include "../Structs/ObjectPool.h"

struct SurfaceNetsGPUBuffer {
    GLuint ssboSDF = 0;
    GLuint ssboPositions = 0;
    GLuint ssboNormals = 0;
    GLuint ssboIndices = 0;
    GLuint ssboStrideToIndex = 0;
    GLuint ssboCounters = 0;

    // Tamanhos atualmente alocados (em bytes) 
    size_t allocatedVoxels = 0; // quantidade de voxels atualmente alocada (voxelCount)
    bool inUse = false;

    // meta info para cleanup / LRU
    std::chrono::steady_clock::time_point lastUsed = std::chrono::steady_clock::now();


    // helper
    void ensureCapacity(size_t voxelCount) {
        if (allocatedVoxels >= voxelCount) return;
        size_t newCount = std::max(voxelCount, allocatedVoxels * 2);
        if (newCount < 1) newCount = voxelCount;
        // aloca/realoca cada SSBO (note: chamadas GL precisam de contexto atual nesta thread)
        if (!ssboSDF) glGenBuffers(1, &ssboSDF);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboSDF);
        glBufferData(GL_SHADER_STORAGE_BUFFER, newCount * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

        if (!ssboPositions) glGenBuffers(1, &ssboPositions);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPositions);
        glBufferData(GL_SHADER_STORAGE_BUFFER, newCount * sizeof(glm::vec4), nullptr, GL_DYNAMIC_COPY);

        if (!ssboNormals) glGenBuffers(1, &ssboNormals);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNormals);
        glBufferData(GL_SHADER_STORAGE_BUFFER, newCount * sizeof(glm::vec4), nullptr, GL_DYNAMIC_COPY);

        if (!ssboIndices) glGenBuffers(1, &ssboIndices);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndices);
        glBufferData(GL_SHADER_STORAGE_BUFFER, newCount * 6u * sizeof(uint32_t), nullptr, GL_DYNAMIC_COPY);

        if (!ssboStrideToIndex) glGenBuffers(1, &ssboStrideToIndex);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboStrideToIndex);
        glBufferData(GL_SHADER_STORAGE_BUFFER, newCount * sizeof(uint32_t), nullptr, GL_DYNAMIC_COPY);

        if (!ssboCounters) glGenBuffers(1, &ssboCounters);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCounters);
        glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(uint32_t), nullptr, GL_DYNAMIC_COPY);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        allocatedVoxels = newCount;
    }

    void destroy() {
        if (ssboSDF) glDeleteBuffers(1, &ssboSDF);
        if (ssboPositions) glDeleteBuffers(1, &ssboPositions);
        if (ssboNormals) glDeleteBuffers(1, &ssboNormals);
        if (ssboIndices) glDeleteBuffers(1, &ssboIndices);
        if (ssboStrideToIndex) glDeleteBuffers(1, &ssboStrideToIndex);
        if (ssboCounters) glDeleteBuffers(1, &ssboCounters);
        *this = SurfaceNetsGPUBuffer();
    }

};
// Create/Destroy/Reset lambdas
inline SurfaceNetsGPUBuffer* CreateSurfaceBuffer() { return new SurfaceNetsGPUBuffer(); }
inline void DestroySurfaceBuffer(SurfaceNetsGPUBuffer* b) { if (b) { b->destroy(); delete b; } }
inline void ResetSurfaceBuffer(SurfaceNetsGPUBuffer* b) { /* no-op */ }

// Convenience global pool (use DECLARE/DEFINE macros in your code)
using SNBufferPool = ObjectPool<SurfaceNetsGPUBuffer>;