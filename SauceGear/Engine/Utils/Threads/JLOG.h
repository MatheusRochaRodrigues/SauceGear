#pragma once
#include<iostream>

#define JOB_DEBUG 1

#if JOB_DEBUG
#define JOB_LOG(x) std::cout << x << std::endl;
#else
#define JOB_LOG(x)
#endif

/*
JOB_LOG("[SCHEDULE] Task " << t);
JOB_LOG("[EXEC] Task " << current);
JOB_LOG("[FINISH] Task " << task);
JOB_LOG("[DEPENDENCY] " << parent << " -> " << child);
*/