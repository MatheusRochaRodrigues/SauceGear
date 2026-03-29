#pragma once
#include "../Components/TransformComponent.h"
#include "../Components/HierarchyComponent.h"
#include "../Scene/SceneECS.h"
#include "../Core/EngineContext.h"
#include "../System.h"  

// Sistema ECS que roda no começo do frame
class EndLoopSystem : public System {
public:  
    void Update(float deltaTime) override {
        try {
            UpdateTransforms();

            UpdateCleanupSystem();
        }
        catch (const std::exception& e) {
            std::cerr << "[EXCEÇÃO - EndLoopSystem] " << e.what() << "\n";
        }
    }

    std::vector<Entity>* toDestroy;

private:
    // Destroy GameObjects
    void UpdateCleanupSystem() { 
        for (auto e : *toDestroy)
            GEngine->scene->DestroyEntity(e);

        toDestroy->clear();  
    }

    // TRANSFORMS
    void UpdateTransforms() { 
        auto entities = GEngine->scene->GetEntitiesWith<TransformComponent>();
        for (Entity e : entities) GEngine->scene->GetComponent<TransformComponent>(e).transformChangedThisFrame = false;
    }
};

