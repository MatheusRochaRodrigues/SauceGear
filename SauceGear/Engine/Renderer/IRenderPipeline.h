#pragma once
#include "../Scene/SceneECS.h" 

class IRenderPipeline {
public:
    virtual ~IRenderPipeline() = default;
    virtual void Init() = 0;
    virtual void Render(SceneECS& scene) = 0;
    virtual void Shutdown() = 0;
};

