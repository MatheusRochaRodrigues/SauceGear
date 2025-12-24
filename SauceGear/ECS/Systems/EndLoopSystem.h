#pragma once
#include "../Components/Transform.h"
#include "../Components/HierarchyComponent.h"
#include "../Scene/SceneECS.h"
#include "../Core/EngineContext.h"
#include "../System.h" 


// Sistema ECS que roda no começo do frame
class EndLoopSystem : public System {
public:  
    void Update(float deltaTime) override {
        auto entities = GEngine->scene->GetEntitiesWith<Transform>();
        try {
            for (Entity e : entities) GEngine->scene->GetComponent<Transform>(e).transformChangedThisFrame = false;

        } catch (const std::exception& e) {
            std::cerr << "[EXCEÇÃO - DebugRenderer] " << e.what() << "\n";
        }
    }
};