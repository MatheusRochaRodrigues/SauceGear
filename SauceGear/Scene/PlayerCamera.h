#pragma once

#include "ScriptBehaviour.h"
#include "../../Core/EngineContext.h"  
#include "SceneECS.h"   
#include "Components/ComponentsHelper.h"  
#include "../Core/Input.h"


class PlayerCamera : public ScriptBehaviour {
public:
    void Update(float dt) override {
        auto& cam = GEngine->scene->GetComponent<CameraComponent>(entity);
        if (Input::GetKey(KEY_W)) {
            //cam.camera->ProcessKeyboard(FORWARD, dt);
            std::cout << "lelo";
        }
        // e assim por diante...
    }
};
