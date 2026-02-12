#pragma once
#include "Profiler.h"
#include <chrono>

class ScopedCPUProfiler {
public:
    ScopedCPUProfiler(const char* name)
        : name(name),
        start(std::chrono::high_resolution_clock::now()) {
    }

    ~ScopedCPUProfiler() {
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        Profiler::Get().AddCPU(name, ms);
    }

private:
    const char* name;
    std::chrono::high_resolution_clock::time_point start;
};
