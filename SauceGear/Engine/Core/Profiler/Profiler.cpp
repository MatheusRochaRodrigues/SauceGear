#include "Profiler.h" 
#include "FrameMetrics.h" 
#include "../Time.h" 

Profiler& Profiler::Get() {
    static Profiler instance;
    return instance;
}  

void Profiler::BeginFrame() {
    currentFrame = (currentFrame + 1) % PROFILER_GPU_LATENCY;

    for (auto& [_, s] : samples) s.ResetFrame(); 

    //Metricts Stats
    auto& metrics = g_FrameMetrics.Write();
    metrics.ResetCounters(); 

}


void Profiler::EndFrame() {
    int readFrame = (currentFrame + 1) % PROFILER_GPU_LATENCY;
    auto& queries = gpuQueries[readFrame];

    for (auto& q : queries) {
        GLuint available = 0;
        glGetQueryObjectuiv(q.id, GL_QUERY_RESULT_AVAILABLE, &available);

        if (!available)
            continue;

        GLuint64 time = 0;
        glGetQueryObjectui64v(q.id, GL_QUERY_RESULT, &time);

        samples[q.name].gpuMs += double(time) / 1e6;
        samples[q.name].hits++;

        glDeleteQueries(1, &q.id);
    }

    queries.clear();

    for (auto& [_, s] : samples)
        s.PushHistory();

     
    auto& metrics = g_FrameMetrics.Write(); 
    metrics.Update(Time::GetDeltaTime());

    g_FrameMetrics.Swap();
}


void Profiler::AddCPU(const std::string& name, double ms) {
    auto& s = samples[name];
    s.cpuMs += ms;
    s.hits++;
}

void Profiler::SubmitGPU(const std::string& name, GLuint query) {
    gpuQueries[currentFrame].push_back({ query, name });
}

const std::unordered_map<std::string, ProfileSample>&
Profiler::GetSamples() const {
    return samples;
}
