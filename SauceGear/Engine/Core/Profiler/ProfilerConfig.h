#pragma once 

#if defined(_DEBUG) || defined(PROFILER_ENABLED)
#define ENABLE_PROFILER 1
#else
#define ENABLE_PROFILER 0
#endif
