#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <glad/glad.h>

static constexpr int PROFILER_FRAMES = 3; 

struct ProfileSample {
    double cpuMs = 0.0;
    double gpuMs = 0.0;
    uint32_t hits = 0;
};

struct GPUQuery {
    GLuint id;
    std::string name;
};

class Profiler {
public:
    static Profiler& Get() {
        static Profiler instance;
        return instance;
    }

    void BeginFrame() {
        currentFrame = (currentFrame + 1) % PROFILER_FRAMES;

        frameSamples.clear();
    }

    void EndFrame() {
        auto& queries = gpuQueries[currentFrame]; 
        for (auto& q : queries) {
            GLuint available = 0;
            glGetQueryObjectuiv(q.id, GL_QUERY_RESULT_AVAILABLE, &available);

            if (available) {
                GLuint64 time;
                glGetQueryObjectui64v(q.id, GL_QUERY_RESULT, &time);
                frameSamples[q.name].gpuMs += double(time) / 1e6;
                glDeleteQueries(1, &q.id);
            }
        } 
        queries.clear();

        //CPU
        displaySamples = frameSamples; //  snapshot
    }

    void AddCPU(const std::string& name, double ms) {
        auto& s = frameSamples[name];
        s.cpuMs += ms;
        s.hits++;
    }
     
    void SubmitGPU(const std::string& name, GLuint query) {
        gpuQueries[currentFrame].push_back({ query, name });
    }

    const std::unordered_map<std::string, ProfileSample>& GetFrameSamples() const {
        return displaySamples;
    }

private:
    std::unordered_map<std::string, ProfileSample> displaySamples;
    std::unordered_map<std::string, ProfileSample> frameSamples;

    int currentFrame = 0;
    std::vector<GPUQuery> gpuQueries[PROFILER_FRAMES];
};
