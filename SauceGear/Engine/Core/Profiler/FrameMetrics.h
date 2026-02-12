#pragma once
#include <cstdint>
#include <algorithm>  


struct FrameMetrics {
    // Frame timing
    float frameMs = 0.0f;
    float frameAvg = 0.0f;

    float fps = 0.0f;
    float fpsAvg = 0.0f;

    // Render stats
    uint32_t drawCalls = 0;
    uint32_t triangles = 0;

    void ResetCounters() {
        drawCalls = 0;
        triangles = 0;
    }

    void Update(float deltaSeconds) {
        frameMs = deltaSeconds * 1000.0f;
        fps = deltaSeconds > 0.0f ? (1.0f / deltaSeconds) : 0.0f;

        constexpr float alpha = 0.1f; // EMA smoothing

        if (frameAvg == 0.0f) {
            frameAvg = frameMs;
            fpsAvg = fps;
        }
        else {
            frameAvg = frameAvg * (1.0f - alpha) + frameMs * alpha;
            fpsAvg = fpsAvg * (1.0f - alpha) + fps * alpha;
        }
    }
}; 


struct FrameMetricsDB {
    FrameMetrics buffers[2];
    uint32_t writeIndex = 0;
    uint32_t readIndex = 1;

    FrameMetrics& Write() { return buffers[writeIndex]; }
    const FrameMetrics& Read() const { return buffers[readIndex]; }

    void Swap() {
        std::swap(writeIndex, readIndex);
        //buffers[writeIndex].ResetCounters();
    }
};

extern FrameMetricsDB g_FrameMetrics;