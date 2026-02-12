#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <glad/glad.h>

static constexpr int PROFILER_GPU_LATENCY = 6;
static constexpr int PROFILER_HISTORY = 120;

static constexpr float PROFILER_EMA_ALPHA = 0.1f; // menor = mais estável

struct ProfileSample {
    // frame atual
    double cpuMs = 0.0;
    double gpuMs = 0.0;
    uint32_t hits = 0;

    // histórico
    std::vector<float> cpuHistory;
    std::vector<float> gpuHistory;

    // médias móveis
    float cpuAvg = 0.0f;
    float gpuAvg = 0.0f;

    void ResetFrame() {
        cpuMs = 0.0;
        gpuMs = 0.0;
        hits = 0;
    }

    void PushHistory() {
        if (hits == 0)
            return;

        if (cpuHistory.size() >= PROFILER_HISTORY)
            cpuHistory.erase(cpuHistory.begin());
        if (gpuHistory.size() >= PROFILER_HISTORY)
            gpuHistory.erase(gpuHistory.begin());

        cpuHistory.push_back((float)cpuMs);
        gpuHistory.push_back((float)gpuMs);

        if (cpuHistory.size() == 1) {
            cpuAvg = cpuMs;
            gpuAvg = gpuMs;
        }
        else {
            cpuAvg = cpuAvg * (1.0f - PROFILER_EMA_ALPHA) + cpuMs * PROFILER_EMA_ALPHA;
            gpuAvg = gpuAvg * (1.0f - PROFILER_EMA_ALPHA) + gpuMs * PROFILER_EMA_ALPHA;
        }
    } 

    /*
    void PushHistory() {
        if (hits == 0)
            return;

        if (cpuHistory.size() >= PROFILER_HISTORY)
            cpuHistory.erase(cpuHistory.begin());
        if (gpuHistory.size() >= PROFILER_HISTORY)
            gpuHistory.erase(gpuHistory.begin());

        cpuHistory.push_back((float)cpuMs);
        gpuHistory.push_back((float)gpuMs);

        cpuAvg = 0.0f;
        for (float v : cpuHistory) cpuAvg += v;
        cpuAvg /= cpuHistory.size();

        gpuAvg = 0.0f;
        for (float v : gpuHistory) gpuAvg += v;
        gpuAvg /= gpuHistory.size();
    }
    */


};

struct GPUQuery {
    GLuint id;
    std::string name;
};

class Profiler {
public:
    static Profiler& Get();

    void BeginFrame();
    void EndFrame();

    void AddCPU(const std::string& name, double ms);
    void SubmitGPU(const std::string& name, GLuint query);

    const std::unordered_map<std::string, ProfileSample>& GetSamples() const;

private:
    Profiler() = default;

    int currentFrame = 0;

    std::unordered_map<std::string, ProfileSample> samples;
    std::vector<GPUQuery> gpuQueries[PROFILER_GPU_LATENCY];
};
