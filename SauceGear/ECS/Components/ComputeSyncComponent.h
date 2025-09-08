#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp> 
#include <functional> 

struct ComputeSyncComponent {
    struct PendingSync {
        GLsync sync;
        std::function<void()> onComplete;
        bool completed = false;
    };

    std::vector<PendingSync> syncs;
};

