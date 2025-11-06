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

    static GLuint CreateSSBO(size_t size, GLenum usage = GL_DYNAMIC_COPY, const void* data = nullptr) { 
        GLuint ssbo;
        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
        return ssbo;
    }

    void EnsureSSBO(GLuint& ssbo, size_t size, size_t elementSize, GLenum usage = GL_DYNAMIC_COPY, const void* data = nullptr) {
        size_t newSize = size * elementSize;
        if (!ssbo) glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, newSize, data, usage);
    }

    // helper
    void ensureCapacity(int d = -1) {
        int dim = (d == -1) ? sysv.get_voxelGrid() : d;    //  pointsPerChunk
        if (dim < 1) dim = 1; 
        const size_t dim3 = size_t(dim) * dim * dim;     // voxelCount

        if (allocatedVoxels >= dim3) return;       //size_t newCount = std::max(voxelCount, allocatedVoxels * 2);      estratégia de crescimento amortizado

        // aloca/realoca cada SSBO (note: chamadas GL precisam de contexto atual nesta thread)
         
        EnsureSSBO(ssboSDF,             dim3,       sizeof(float),      GL_DYNAMIC_DRAW); //MapSDF

        EnsureSSBO(ssboPositions,       dim3,       sizeof(glm::vec4),  GL_DYNAMIC_COPY);
        EnsureSSBO(ssboNormals,         dim3,       sizeof(glm::vec4),  GL_DYNAMIC_COPY);
        EnsureSSBO(ssboIndices,         dim3 * 6u,  sizeof(uint32_t),   GL_DYNAMIC_COPY);
        EnsureSSBO(ssboStrideToIndex,   dim3,       sizeof(uint32_t),   GL_DYNAMIC_COPY);
        EnsureSSBO(ssboCounters,        2,          sizeof(uint32_t),   GL_DYNAMIC_COPY);  

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        allocatedVoxels = dim3;
    }

    void destroy() {
        if (ssboSDF) { glDeleteBuffers(1, &ssboSDF); ssboSDF = 0; }
        if (ssboPositions) { glDeleteBuffers(1, &ssboPositions); ssboPositions = 0; }
        if (ssboNormals) { glDeleteBuffers(1, &ssboNormals); ssboNormals = 0; }
        if (ssboIndices) { glDeleteBuffers(1, &ssboIndices); ssboIndices = 0; }
        if (ssboStrideToIndex) { glDeleteBuffers(1, &ssboStrideToIndex); ssboStrideToIndex = 0; }
        if (ssboCounters) { glDeleteBuffers(1, &ssboCounters); ssboCounters = 0; }
        allocatedVoxels = 0;
        inUse = false;
        lastUsed = std::chrono::steady_clock::now();
    }

};
// Create/Destroy/Reset lambdas
inline SurfaceNetsGPUBuffer* CreateSurfaceBuffer() { return new SurfaceNetsGPUBuffer(); }
inline void DestroySurfaceBuffer(SurfaceNetsGPUBuffer* b) { if (b) { b->destroy(); delete b; } }
inline void ResetSurfaceBuffer(SurfaceNetsGPUBuffer* b) { /* no-op */ }

// Convenience global pool (use DECLARE/DEFINE macros in your code)
using SNBufferPool = ObjectPool<SurfaceNetsGPUBuffer>;






/*


    static std::unique_ptr<Mesh> Generate( )
    {
        const int   DimCells  = sysv.get_cellGrid();      // ex: 32
        const int   DimVoxel  = sysv.get_voxelGrid();     // ex: 33
        const float VoxelSize = sysv.get_voxelSize();

        const size_t voxelCount = size_t(DimVoxel) * DimVoxel * DimVoxel;
        assert(voxelCount == buff.densityMap.size());

        // Ensure capacity (GL thread). If not enough, grow.
        //if (gpuBuff.allocatedVoxels < voxelCount) gpuBuff.ensureCapacity(voxelCount);


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
*/