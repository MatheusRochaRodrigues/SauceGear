#pragma once
#include "IPanel.h"
#include "../../Engine/ECS/Components/HierarchyComponent.h" 
#include "../../Engine/Graphics/Renderer.h"  
#include "../../Engine/Scene/SceneBuilder.h"  
#include "../../Engine/Core/KeyCodes.h"  
#include "../../Engine/Resources/Primitives/Primitive.h"  

struct HierarchyPanel : IPanel {  
    void ShowAddMenu(std::shared_ptr<MaterialInstance> defaultMaterial) {
        static char meshToSpawn[32] = "";     // Tipo selecionado para configuração
        static bool showConfigWindow = false; // Janela de parâmetros

        ImVec2 mousePos = ImGui::GetMousePos();

        // Shift + A
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsKeyPressed(ImGuiKey_Tab, true)) {
            ImGui::OpenPopup("AddObjectMenu");
            ImGui::SetNextWindowPos(mousePos, ImGuiCond_Always);
        }

        // Clique direito sobre a Hierarchy (ou qualquer janela)
        // Detecta clique direito sobre a Hierarchy
        //if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("AddObjectMenu");
            ImGui::SetNextWindowPos(mousePos, ImGuiCond_Always);
        }

        // Clique direito sobre a Hierarchy
        //if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight)) {
        //    if (ImGui::MenuItem("Add Object")) {
        //        ImGui::CloseCurrentPopup();        // Fecha o menu do clique direito
        //        ImGui::OpenPopup("AddObjectMenu"); // Marca o outro popup como aberto
        //        ImGui::SetNextWindowPos(mousePos, ImGuiCond_Always);
        //    }
        //    //ImGui::EndPopup();
        //}

        // Menu principal
        if (ImGui::BeginPopup("AddObjectMenu", ImGuiWindowFlags_AlwaysAutoResize)) {

            if (ImGui::BeginMenu("Mesh")) { 
                // Cube spawn direto
                if (ImGui::MenuItem("Cube")) { 
                    SceneBuilder::CreateModel(PrimitiveMesh::Cube());
                    ImGui::CloseCurrentPopup();
                }

                // Sphere → abre janela de configuração
                if (ImGui::MenuItem("Sphere")) {
                    strcpy_s(meshToSpawn, sizeof(meshToSpawn), "Sphere");
                    showConfigWindow = true;
                    ImGui::CloseCurrentPopup();
                }

                // Cylinder → abre janela de configuração
                if (ImGui::MenuItem("Cylinder")) {
                    strcpy_s(meshToSpawn, sizeof(meshToSpawn), "Cylinder"); 
                    showConfigWindow = true;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        }

        // Janela de configuração para Sphere / Cylinder
        if (showConfigWindow) {
            ImGui::Begin("Object Settings", &showConfigWindow, ImGuiWindowFlags_AlwaysAutoResize);

            if (strcmp(meshToSpawn, "Sphere") == 0) {
                static int segments = 16;
                static int rings = 16;
                static float radius = 1.0f;

                ImGui::InputInt("Segments", &segments);
                ImGui::InputInt("Rings", &rings);
                ImGui::InputFloat("Radius", &radius);

                if (ImGui::Button("Spawn Sphere")) { 
                    SceneBuilder::CreateModel(PrimitiveMesh::Sphere(segments, rings, radius));
                    showConfigWindow = false;
                }
            }
            else if (strcmp(meshToSpawn, "Cylinder") == 0) {
                static int segments = 16;
                static float height = 2.0f;
                static float radius = 1.0f;
                static bool capped = true;

                ImGui::InputInt("Segments", &segments);
                ImGui::InputFloat("Height", &height);
                ImGui::InputFloat("Radius", &radius);
                ImGui::Checkbox("Capped", &capped);

                if (ImGui::Button("Spawn Cylinder")) {  
                    SceneBuilder::CreateModel(PrimitiveMesh::Cylinder(segments, height, radius, capped));
                    showConfigWindow = false;
                }
            }

            ImGui::End();
        }
    }



    void Draw(SceneECS& scene) override { 
        ImGui::Begin("Hierarchy");
         
        ShowAddMenu(nullptr);

        //DrawHierarchyPopup("currentDirectory");

        for (Entity entity : scene.GetAllEntities()) {
            // Só renderiza se nao tiver pai (ou seja, é um nó raiz da hierarquia)
            if (scene.HasComponent<HierarchyComponent>(entity)) {
                const auto& hierarchy = scene.GetComponent<HierarchyComponent>(entity);
                if (hierarchy.parent == INVALID_ENTITY) {
                    DrawEntityNode(scene, entity);
                }
            }
            else {
                // Entidades sem HierarchyComponent sao tratadas como raiz também
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

  