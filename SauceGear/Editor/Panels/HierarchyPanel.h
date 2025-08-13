#pragma once
#include "IPanel.h"
#include "../Scene/Components/ComponentsHelper.h" 
#include "../Graphics/Renderer.h"  

struct HierarchyPanel : IPanel {
    void Draw(SceneECS& scene) override { 
        ImGui::Begin("Hierarchy");
        //for (auto entity : scene.GetAllEntities()) {
            //if (ImGui::Selectable(entity.GetName().c_str(), entity == scene.GetSelectedEntity())) {
            /*if (ImGui::Selectable("entity", entity == scene.GetSelectedEntity())) {
                scene.SelectEntity(entity);
            }*/

        for (Entity entity : scene.GetAllEntities()) {
            // Só renderiza se não tiver pai (ou seja, é um nó raiz da hierarquia)
            if (scene.HasComponent<HierarchyComponent>(entity)) {
                const auto& hierarchy = scene.GetComponent<HierarchyComponent>(entity);
                if (hierarchy.parent == INVALID_ENTITY) {
                    DrawEntityNode(scene, entity);
                }
            }
            else {
                // Entidades sem HierarchyComponent são tratadas como raiz também
                DrawEntityNode(scene, entity);
            }
        }
        //}
        ImGui::End(); 
    }

    const char* GetName() override { return "Scene"; }

private:
     
    void DrawEntityNode(SceneECS& scene, Entity entity) {
        const auto& name = scene.HasComponent<NameComponent>(entity) ? scene.GetComponent<NameComponent>(entity).name : "Unnamed Entity";

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (scene.GetSelectedEntity() == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool hasChildren = false;
        if (scene.HasComponent<HierarchyComponent>(entity)) {
            const auto& h = scene.GetComponent<HierarchyComponent>(entity);
            hasChildren = (h.firstChild != INVALID_ENTITY);
        }

        bool open = false;
        if (hasChildren)
            open = ImGui::TreeNodeEx((void*)(intptr_t)entity, flags, "%s", name.c_str());
        else
            ImGui::TreeNodeEx((void*)(intptr_t)entity, flags | ImGuiTreeNodeFlags_Leaf, "%s", name.c_str());

        if (ImGui::IsItemClicked())
            scene.SelectEntity(entity);

        // Desenha filhos recursivamente
        if (open && hasChildren) {
            Entity child = scene.GetComponent<HierarchyComponent>(entity).firstChild;
            while (child != INVALID_ENTITY) {
                DrawEntityNode(scene, child);
                child = scene.GetComponent<HierarchyComponent>(child).nextSibling;
            }
            ImGui::TreePop();
        }

        if (!hasChildren)
            ImGui::TreePop(); // necessário para manter o estado correto se não tiver filhos
    }

     
};

