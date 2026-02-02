#pragma once
#include "IPanel.h"
#include "../../Engine/ECS/Components/HierarchyComponent.h" 
#include "../../Engine/Graphics/Renderer.h"  
#include "../../Engine/Scene/SceneBuilder.h"  
#include "../../Engine/Core/KeyCodes.h"  
#include "../../Engine/Resources/Primitives/Primitive.h"  

struct HierarchyPanel : IPanel {   

    //void Draw(SceneECS& scene) override { 
    //    ImGui::Begin("Hierarchy");
    //     
    //    ShowAddMenu(nullptr);

    //    HandleShortcuts(scene);
    //    //DrawHierarchyPopup("currentDirectory");

    //    for (Entity entity : scene.GetAllEntities()) {
    //        // Só renderiza se nao tiver pai (ou seja, é um nó raiz da hierarquia)
    //        if (scene.HasComponent<HierarchyComponent>(entity)) {
    //            const auto& hierarchy = scene.GetComponent<HierarchyComponent>(entity);
    //            if (hierarchy.parent == INVALID_ENTITY) {
    //                DrawEntityNode(scene, entity);
    //            }
    //        }
    //        else {
    //            // Entidades sem HierarchyComponent sao tratadas como raiz também
    //            DrawEntityNode(scene, entity);
    //        }
    //    } 
    //    ImGui::End(); 
    //}

    void Draw(SceneECS& scene) override {
        ImGui::Begin("Hierarchy"); 

        HandleShortcuts(scene);

        DrawAddObjectPopup();

        for (Entity entity : scene.GetAllEntities()) {
            if (IsRoot(scene, entity)) {
                DrawEntityNode(scene, entity);
            }
        }

        ImGui::End();
    }

    const char* GetName() override { return "Scene"; }

private: 
    // ===============================
    // CONTEXT MENU
    // ===============================
    void DrawContextMenu(SceneECS& scene, Entity entity) {

        ImGui::PushID((int)entity); // ID ÚNICO POR ENTIDADE

        if (ImGui::BeginPopupContextItem()) {

            if (ImGui::MenuItem("Add Object")) {
                ImGui::OpenPopup("AddObjectMenu");
            }

            if (ImGui::MenuItem("Delete")) {
                scene.DestroyEntity(entity);
                ImGui::EndPopup();
                ImGui::PopID();
                return;
            }

            ImGui::Separator(); 

            ImGui::EndPopup();
        } 

        ImGui::PopID();
    }


    void DrawEntityNode(SceneECS& scene, Entity entity) {

        const char* label =
            scene.HasComponent<NameComponent>(entity)
            ? scene.GetComponent<NameComponent>(entity).name.c_str()
            : "Entity";

        bool hasChildren = false;
        if (scene.HasComponent<HierarchyComponent>(entity)) {
            hasChildren = scene.GetComponent<HierarchyComponent>(entity).firstChild != INVALID_ENTITY;
        }

        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth;

        if (scene.GetSelectedEntity() == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        if (!hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf;

        bool open = ImGui::TreeNodeEx(
            (void*)(intptr_t)entity,
            flags,
            "%s",
            label
        );

        // seleção
        if (ImGui::IsItemClicked()) {
            scene.SelectEntity(entity);
        }

        // contexto
        DrawContextMenu(scene, entity);
         
        if (open) {
            if (hasChildren) {
                Entity child = scene.GetComponent<HierarchyComponent>(entity).firstChild;
                while (child != INVALID_ENTITY) {
                    DrawEntityNode(scene, child);
                    child = scene.GetComponent<HierarchyComponent>(child).nextSibling;
                }
            }
            ImGui::TreePop();
        }
    }



    // ===============================
    // INPUT
    // ===============================
    void HandleShortcuts(SceneECS& scene) {
        if (scene.GetSelectedEntity() != INVALID_ENTITY &&
            ImGui::IsWindowFocused() &&
            ImGui::IsKeyPressed(ImGuiKey_Delete)) {

            scene.DestroyEntity(scene.GetSelectedEntity());
        }
    }

    // ===============================
   // HIERARCHY
   // ===============================
    bool IsRoot(SceneECS& scene, Entity e) {
        if (!scene.HasComponent<HierarchyComponent>(e))
            return true;

        return scene.GetComponent<HierarchyComponent>(e).parent == INVALID_ENTITY;
    }

private: 

    void DrawAddObjectPopup() {
        static char meshToSpawn[32] = "";     // Tipo selecionado para configuração
        static bool showConfigWindow = false; // Janela de parâmetros 
        ImVec2 mousePos = ImGui::GetMousePos();

        // Shift + A
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsKeyPressed(ImGuiKey_Tab, true)) {
            ImGui::OpenPopup("AddObjectMenu");
            ImGui::SetNextWindowPos(mousePos, ImGuiCond_Always);
        }

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

                // Cylinder → abre janela de configuração
                if (ImGui::MenuItem("Plane")) {
                    auto e = SceneBuilder::CreateModel(PrimitiveMesh::Plane());
                    scn->AddComponent<NameComponent>(e).name = "Plane";
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Ligth")) {
                    auto e = SceneBuilder::CreateLight();
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
};

  
/*

if (ImGui::BeginPopupContextItem()) {

    if (ImGui::BeginMenu("Add Object")) {

        if (ImGui::MenuItem("Cube")) {
            SceneBuilder::CreateModel(PrimitiveMesh::Cube());
        }

        if (ImGui::MenuItem("Sphere")) {
            showConfigWindow = true;
        }

        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Delete")) {
        scene.DestroyEntity(entity);
    }

    ImGui::EndPopup();
}



*/



/*

CONTEXT MENU + SUBMENUS (seu caso)

Exemplo igual Unity / Unreal:

if (ImGui::BeginPopupContextWindow()) {

    if (ImGui::BeginMenu("Add")) {

        if (ImGui::BeginMenu("Mesh")) {
            ImGui::MenuItem("Cube");
            ImGui::MenuItem("Sphere");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Light")) {
            ImGui::MenuItem("Directional");
            ImGui::MenuItem("Point");
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    ImGui::EndPopup();
}




if (ImGui::BeginMenu("Add Object")) {

    if (ImGui::BeginMenu("Mesh")) {

        if (ImGui::MenuItem("Cube")) {
            // ação
        }

        if (ImGui::BeginMenu("Sphere")) {

            if (ImGui::MenuItem("Low Poly")) {
                // cria esfera low
            }

            if (ImGui::MenuItem("High Poly")) {
                // cria esfera high
            }

            ImGui::EndMenu(); // Sphere
        }

        ImGui::EndMenu(); // Mesh
    }

    if (ImGui::BeginMenu("Light")) {

        if (ImGui::MenuItem("Directional")) {}
        if (ImGui::MenuItem("Point")) {}
        if (ImGui::MenuItem("Spot")) {}

        ImGui::EndMenu(); // Light
    }

    ImGui::EndMenu(); // Add Object
}


*/



/*
void DrawEntityNode(SceneECS& scene, Entity entity) {
        const char* label =
            scene.HasComponent<NameComponent>(entity)
            ? scene.GetComponent<NameComponent>(entity).name.c_str()
            : "Entity";

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
            open = ImGui::TreeNodeEx((void*)(intptr_t)entity, flags, "%s", label);
        else
            ImGui::TreeNodeEx((void*)(intptr_t)entity, flags | ImGuiTreeNodeFlags_Leaf, "%s", label);

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
*/