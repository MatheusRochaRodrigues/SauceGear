#pragma once
#include "Profiler.h"

class ScopedGPUProfiler {
public:
    ScopedGPUProfiler(const char* name)
        : name(name) {
        glGenQueries(1, &query);
        glBeginQuery(GL_TIME_ELAPSED, query);
    }

    ~ScopedGPUProfiler() {
        glEndQuery(GL_TIME_ELAPSED);
        Profiler::Get().SubmitGPU(name, query);
    }

private:
    const char* name;
    GLuint query;
};
