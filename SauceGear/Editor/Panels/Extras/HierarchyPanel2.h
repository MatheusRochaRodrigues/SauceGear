#pragma once
#include "IPanel.h"
#include "../ECS/Components/ComponentsHelper.h" 
#include "../Graphics/Renderer.h"  

struct HierarchyPanel : IPanel {

    void popup() {
        // Clique com botão direito em qualquer área vazia da janela
        if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::MenuItem("Import", "Ctrl+I")) {
                // TODO: Abrir diálogo de import
            }
            if (ImGui::MenuItem("Cut", "Ctrl+X")) {
                // TODO: Cortar entidade selecionada
            }
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                // TODO: Copiar entidade selecionada
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                // TODO: Colar entidade
            }

            if (ImGui::BeginMenu("Add New")) {
                if (ImGui::MenuItem("Folder", "Ctrl+Shift+N")) {
                    // TODO: Criar nova pasta
                }
                if (ImGui::MenuItem("Scene")) {
                    // TODO: Criar nova cena
                }
                if (ImGui::MenuItem("Prefab")) {
                    // TODO: Criar prefab
                }
                if (ImGui::MenuItem("Material")) {
                    // TODO: Criar material
                }
                if (ImGui::MenuItem("Texture")) {
                    // TODO: Criar textura
                }
                if (ImGui::MenuItem("C# Script")) {
                    // TODO: Criar script
                }
                if (ImGui::MenuItem("Animation")) {
                    // TODO: Criar animação
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Refresh", "Ctrl+R")) {
                // TODO: Recarregar hierarquia
            }
            if (ImGui::MenuItem("Open in Explorer")) {
                //system(("explorer " + currentDirectory).c_str()); // Windows
            }

            ImGui::EndPopup();
        }
    }

    void Draw(SceneECS& scene) override { 
        ImGui::Begin("Hierarchy");

        popup();

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








//for (auto entity : scene.GetAllEntities()) {
    //if (ImGui::Selectable(entity.GetName().c_str(), entity == scene.GetSelectedEntity())) {
    /*if (ImGui::Selectable("entity", entity == scene.GetSelectedEntity())) {
        scene.SelectEntity(entity);
    }*/