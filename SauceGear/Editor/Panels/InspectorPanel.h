#pragma once
#include "IPanel.h"
#include "../Utils/InspectorDrawer.h"
#include "../Scene/SceneECS.h"

struct InspectorPanel : IPanel {
    void Draw(SceneECS& scene) override {
        ImGui::Begin("Inspector");

        Entity selected = scene.GetSelectedEntity();
        if (selected != INVALID_ENTITY) {
            InspectorDrawer::DrawEntity(scene, selected);
        }

        ImGui::End();
    }

    const char* GetName() override { return "Inspector"; }
};