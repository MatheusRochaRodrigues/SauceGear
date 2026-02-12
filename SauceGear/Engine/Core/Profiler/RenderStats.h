#include <atomic>

struct RenderStats {
    uint32_t drawCalls = 0;
    uint32_t triangles = 0;

    void Reset() {
        drawCalls = 0;
        triangles = 0;
    }
};

struct RenderStatsDB {
    RenderStats buffers[2];
    uint32_t writeIndex = 0;
    uint32_t readIndex = 1;

    RenderStats& Write() { return buffers[writeIndex]; }
    const RenderStats& Read() const { return buffers[readIndex]; }

    void Swap() {
        std::swap(writeIndex, readIndex);
        buffers[writeIndex].Reset();
    }
}; 
 
extern RenderStatsDB g_RenderStats;