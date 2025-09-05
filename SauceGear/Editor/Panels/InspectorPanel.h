#pragma once
#include "IPanel.h"
#include "../Utils/InspectorDrawer.h"
#include "../Scene/SceneECS.h"
#include <imgui.h>
#include "../ImGui/Fonts/IconsFontAwesome5.h"

struct InspectorPanel : IPanel {
    void Draw(SceneECS& scene) override {
        ImGui::Begin("Inspector");

        Entity selected = scene.GetSelectedEntity();
        if (selected == INVALID_ENTITY) {
            ImGui::TextDisabled("No entity selected");
            ImGui::End();
            return;
        }

        for (auto* typeInfo : scene.GetComponentTypes(selected)) {
            void* compPtr = scene.GetComponentRaw(selected, *typeInfo);
            if (!compPtr) continue;

            // ===== HEADER ESTILIZADO =====
            ImGuiTreeNodeFlags nodeFlags =
                ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth;

            // Altura maior para o header
            float headerHeight = 28.0f; // pode ajustar
            ImVec2 framePadding(8, (headerHeight - ImGui::GetFontSize()) * 0.5f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);

            // Ícone + nome do componente
            std::string header = ICON_FA_CUBE;
            header += " ";
            header += typeInfo->name;

            bool open = ImGui::TreeNodeEx((void*)typeInfo, nodeFlags, "%s", header.c_str());

            // ===== BOTÃO X =====
            float buttonSize = ImGui::GetFontSize() * 1.2f;
            ImVec2 lastItemRectMin = ImGui::GetItemRectMin();
            ImVec2 lastItemRectMax = ImGui::GetItemRectMax();

            ImGui::SetCursorScreenPos(ImVec2(
                lastItemRectMax.x - buttonSize - 6, // 6px margin da borda
                lastItemRectMin.y + (headerHeight - buttonSize) * 0.5f
            ));

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
            if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
                scene.RemoveComponent(selected, *typeInfo);
                if (open) ImGui::TreePop();
                ImGui::PopStyleVar(); // botão
                ImGui::PopStyleVar(); // header
                continue;
            }
            ImGui::PopStyleVar(); // botão

            // ===== PROPRIEDADES =====
            if (open) {
                ImGui::Indent(10);
                for (auto& field : typeInfo->fields) {
                    InspectorDrawer::DrawProperty(field, compPtr);
                }
                ImGui::Unindent(10);
                ImGui::TreePop();
            }

            ImGui::PopStyleVar(); // header
            ImGui::Separator();   // linha separadora
        }

        ImGui::End();
    }

    const char* GetName() override { return "Inspector"; }
};







//void Draw(SceneECS& scene) override {
//    ImGui::Begin("Inspector");
//
//    Entity selected = scene.GetSelectedEntity();
//    if (selected == INVALID_ENTITY) {
//        ImGui::TextDisabled("No entity selected");
//        ImGui::End();
//        return;
//    }
//
//    for (auto* typeInfo : scene.GetComponentTypes(selected)) {
//        void* compPtr = scene.GetComponentRaw(selected, *typeInfo);
//        if (!compPtr) continue;
//
//        // ---------- Custom Component Header ----------
//        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed
//            | ImGuiTreeNodeFlags_AllowItemOverlap
//            | ImGuiTreeNodeFlags_SpanAvailWidth
//            | ImGuiTreeNodeFlags_FramePadding;
//
//        // Ícone Unicode ou pequeno ImGuiImage (pode trocar por textura futura)
//        std::string header = ICON_FA_CUBE; // exemplo de ícone FontAwesome, pode trocar
//        header += " ";
//        header += typeInfo->name;
//
//        bool open = ImGui::TreeNodeEx((void*)typeInfo, nodeFlags, "%s", header.c_str());
//
//        // Botão de remover no canto direito
//        ImGui::SameLine(ImGui::GetContentRegionAvail().x);
//        if (ImGui::SmallButton("[X]")) {
//            scene.RemoveComponent(selected, *typeInfo);
//            if (open) ImGui::TreePop();
//            continue;
//        }
//
//        // ---------- Component Properties ----------
//        if (open) {
//            ImGui::Indent(10);
//            for (auto& field : typeInfo->fields) {
//                InspectorDrawer::DrawProperty(field, compPtr);
//            }
//            ImGui::Unindent(10);
//            ImGui::TreePop();
//        }
//
//        ImGui::Separator(); // linha separadora entre componentes
//    }
//
//    ImGui::End();
//}