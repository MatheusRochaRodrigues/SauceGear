#pragma once
#include <vector> 
#include <glad/glad.h>
#include <iostream>
#include <cassert>
#include "SurfaceNets.h"

// =========================================================
// Estruturas e funções utilitárias
// =========================================================

struct SurfaceNetsGPUBuffer {
    GLuint ssboSDF = 0;
    GLuint ssboPositions = 0;
    GLuint ssboNormals = 0;
    GLuint ssboIndices = 0;
    GLuint ssboStrideToIndex = 0;
    GLuint ssboCounters = 0;
    size_t allocatedVoxels = 0;
    bool inUse = false;
};

static void EnsureSSBO(GLuint& ssbo, size_t currentCount, size_t newCount, size_t elementSize, GLenum usage = GL_DYNAMIC_COPY) {
    size_t newSize = newCount * elementSize;
    size_t oldSize = currentCount * elementSize;

    if (ssbo == 0) {
        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, newSize, nullptr, usage);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    else if (newSize > oldSize) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, newSize, nullptr, usage);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

// =========================================================
// Pool de buffers SurfaceNets
// =========================================================

class SurfaceNetsBufferPool {
public:
    SurfaceNetsBufferPool() = default;

    ~SurfaceNetsBufferPool() {
        for (auto& buf : pool) {
            DestroyBuffer(buf);
        }
    }

    SurfaceNetsGPUBuffer* Get(size_t voxelCount) {
        // tenta achar um buffer livre e suficiente
        for (auto& buf : pool) {
            if (!buf.inUse && buf.allocatedVoxels >= voxelCount) {
                buf.inUse = true;
                return &buf;
            }
        }

        // nenhum disponível → cria novo
        SurfaceNetsGPUBuffer newBuf;
        newBuf.inUse = true;
        newBuf.allocatedVoxels = voxelCount;
        AllocateBuffers(newBuf, voxelCount);
        pool.push_back(std::move(newBuf));
        return &pool.back();
    }

    void Release(SurfaceNetsGPUBuffer* buf) {
        if (buf) buf->inUse = false;
    }

    void Clear() {
        for (auto& buf : pool) DestroyBuffer(buf);
        pool.clear();
    }

private:
    std::vector<SurfaceNetsGPUBuffer> pool;

    void AllocateBuffers(SurfaceNetsGPUBuffer& gpu, size_t voxelCount) {
        EnsureSSBO(gpu.ssboSDF, 0, voxelCount, sizeof(float), GL_DYNAMIC_DRAW);
        EnsureSSBO(gpu.ssboPositions, 0, voxelCount, sizeof(glm::vec4), GL_DYNAMIC_COPY);
        EnsureSSBO(gpu.ssboNormals, 0, voxelCount, sizeof(glm::vec4), GL_DYNAMIC_COPY);
        EnsureSSBO(gpu.ssboIndices, 0, voxelCount * 6u, sizeof(uint32_t), GL_DYNAMIC_COPY);
        EnsureSSBO(gpu.ssboStrideToIndex, 0, voxelCount, sizeof(uint32_t), GL_DYNAMIC_COPY);
        EnsureSSBO(gpu.ssboCounters, 0, 2, sizeof(uint32_t), GL_DYNAMIC_COPY);
    }

    void DestroyBuffer(SurfaceNetsGPUBuffer& gpu) {
        GLuint bufs[] = {
            gpu.ssboSDF, gpu.ssboPositions, gpu.ssboNormals,
            gpu.ssboIndices, gpu.ssboStrideToIndex, gpu.ssboCounters
        };
        glDeleteBuffers(6, bufs);
        memset(&gpu, 0, sizeof(gpu));
    }
};
