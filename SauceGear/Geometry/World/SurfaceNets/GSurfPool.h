#pragma once
#include "SurfaceNetsGPUBuffer.h"
#include "../Structs/ObjectPool.h"
#include <functional>
#include <memory>

//-------------------------------------------------
//------singleton wrapper around ObjectPool--------
//-------------------------------------------------

// forward: Create/Destroy/Reset used by ObjectPool
inline SurfaceNetsGPUBuffer* SN_Create() {
    return new SurfaceNetsGPUBuffer();
}
inline void SN_Destroy(SurfaceNetsGPUBuffer* b) {
    if (!b) return;
    // NOTE: GL calls must happen on GL thread/context
    b->destroy();
    delete b;
}
inline void SN_Reset(SurfaceNetsGPUBuffer* b) {
    if (!b) return;
    b->inUse = false;
    b->lastUsed = std::chrono::steady_clock::now();
}

class GlobalSurfaceNetsPool {
public:
    using PoolType = ObjectPool<SurfaceNetsGPUBuffer>;

    static GlobalSurfaceNetsPool& Get() {
        static GlobalSurfaceNetsPool instance;
        return instance;
    }

    // Acquire a buffer that can hold voxelCount; will call ensureCapacity on GL thread later
    SurfaceNetsGPUBuffer* Acquire() {
        const int pointsPerChunk = sysv.get_voxelGrid();
        const size_t voxelCount = size_t(pointsPerChunk) * pointsPerChunk * pointsPerChunk;

        SurfaceNetsGPUBuffer* b = pool.acquire();
        if (!b) {
            // pool saturated - try to create directly (shouldn't normally happen)
            b = SN_Create();
        }
        // mark inUse and set desired capacity; real GL reallocation must be done on GL context thread
        b->inUse = true;

        // store desired minimal voxels in allocatedVoxels temporarily if smaller; caller should call ensureCapacity on GL thread
        //if (b->allocatedVoxels < voxelCount) {
            //b->allocatedVoxels = voxelCount;  // ✅ real fix
            // don't call GL from here; caller will call ensureCapacity before using
            //b->allocatedVoxels = b->allocatedVoxels; // no-op placeholder
        //}

        b->lastUsed = std::chrono::steady_clock::now();
        return b;
    }

    void Release(SurfaceNetsGPUBuffer* b) {
        if (!b) return;
        b->inUse = false;
        b->lastUsed = std::chrono::steady_clock::now();
        pool.release(b);
    }

    void CleanupIdle() { pool.cleanupIdle(); }
    void ClearAll() { pool.clearAll(); }

private:
    GlobalSurfaceNetsPool() :
        pool(SN_Create, SN_Destroy, SN_Reset, typename PoolType::Config{}) {
    }
    ~GlobalSurfaceNetsPool() = default;

    PoolType pool;
};

/*
Nota: GlobalSurfaceNetsPool::Acquire retorna um buffer; 
antes de usar você deve chamar gpuBuf->ensureCapacity(voxelCount) na thread com contexto GL(o código abaixo faz isso).
Isso evita GL calls em threads sem contexto.
*/