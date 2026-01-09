#include "EngineContext.h"
#include "EditorState.h" 

#include "../Scene/SceneECS.h" 
#include "../ECS/Components/CameraComponent.h"    // for CameraComponent

EngineContext* GEngine = nullptr;

Camera* EngineContext::GetMainCamera() {
    auto entities = GEngine->scene->GetEntitiesWith<CameraComponent>();
    for (auto e : entities) {
        auto& cam = GEngine->scene->GetComponent<CameraComponent>(e);
        if (cam.isMain) return cam.camera;
    }
    return nullptr;
}


 