#pragma once
#include "ProfilerConfig.h"

#if ENABLE_PROFILER
#include "ScopedCPUProfiler.h"
#include "ScopedGPUProfiler.h"
#define PROFILE_CPU(name) ScopedCPUProfiler _cpu_##__LINE__(name)
#define PROFILE_GPU(name) ScopedGPUProfiler _gpu_##__LINE__(name)
#else
#define PROFILE_CPU(name)
#define PROFILE_GPU(name)
#endif
