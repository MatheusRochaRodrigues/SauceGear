#pragma once

#include "../../Core/EngineContext.h"  
#include "../SceneECS.h"  
#include "../System.h"
#include "../Components/NativeScriptComponent.h"

class ScriptSystem : public System {
public:
    void Update(float dt) override {
        auto entities = GEngine->scene->GetEntitiesWith<NativeScriptComponent>();

        for (auto e : entities) {
            auto& scriptComp = GEngine->scene->GetComponent<NativeScriptComponent>(e);
            if (!scriptComp.instance) {
                scriptComp.instance = scriptComp.InstantiateScript();
                scriptComp.instance->entity = e;
                scriptComp.instance->Start();
            }

            scriptComp.instance->Update(dt);
        }
    }
};
