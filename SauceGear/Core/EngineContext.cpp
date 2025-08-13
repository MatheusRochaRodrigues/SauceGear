#include "EngineContext.h"

#include "../Scene/SceneECS.h"
#include "../Graphics/Renderer.h"
#include "../Platform/Window.h"
#include "Time.h"
#include "InputSystem.h"   
#include "../Scene/SceneManager.h"

#include "../Scene/GameScene.h"   
#include "../Scene/Systems/SystemHelper.h"   

EngineContext* GEngine = nullptr;

Camera* EngineContext::GetMainCamera() {
    auto entities = GEngine->scene->GetEntitiesWith<CameraComponent>();
    for (auto e : entities) {
        auto& cam = GEngine->scene->GetComponent<CameraComponent>(e);
        if (cam.isMain) return cam.camera;
    }
    return nullptr;
}


 