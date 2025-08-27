#pragma once
#include "IPanel.h"
#include "../Scene/Components/ComponentsHelper.h" 
#include "../Graphics/Renderer.h"  
#include "../Core/Time.h"
#include "../Core/EngineContext.h"

struct InspectorPanel : IPanel { 
    void Draw(SceneECS& scene) override {
        ImGui::Begin("Inspector"); 
        ImGui::Text("FPS == %.1f", GEngine->time->GetFPS()); 
        Entity selected = scene.GetSelectedEntity();
        if (selected != INVALID_ENTITY) {
            //DrawComponents(selected, scene);
        }
        ImGui::End();

    }

    const char* GetName() override { return "Scene"; }
     
};

