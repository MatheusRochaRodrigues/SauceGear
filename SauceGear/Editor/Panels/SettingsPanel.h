#pragma once 
#include "IPanel.h"
#include "../Utils/InspectorDrawer.h"
#include "../../Engine/Scene/SceneECS.h"
#include "../../../Engine/Renderer/RenderDebugSettings.h"
#include <imgui.h>

struct SettingsPanel : IPanel {

    void Draw(SceneECS&) override {
        ImGui::Begin("Settings");

        auto& settings = GetEngineSettings();

        // Usa o MESMO sistema de reflection
        TypeInfo* type = ReflectionRegistry::Get().Get(typeid(RenderDebugSettings));
        if (type) {
            InspectorDrawer::DrawType(
                "Render Debug",
                &settings.renderDebug,
                type
            );
        }

        ImGui::End();
    }

    const char* GetName() override { return "Settings"; }
};
