#pragma once
#include "IPanel.h"
#include "../ECS/Components/ComponentsHelper.h" 
#include "../Graphics/Renderer.h"  
#include "Load.h"

#include "../../ECS/Systems/TransformSystem.h" 

struct SceneViewPanel : IPanel {
    SceneViewPanel() { 
        iconTranslate = guiLoadTexture("assets/icons/Translate.png");
        iconRotate    = guiLoadTexture("assets/icons/Rotate.png");
        iconScale     = guiLoadTexture("assets/icons/Scale.png");
    }

    void Draw(SceneECS& scene) override {
        ImGui::Begin("Scene");
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        //verify size buffer
        if (viewportSize.x != lastSize.x || viewportSize.y != lastSize.y) {
            //std::cout << viewportSize.x << " " << viewportSize.y << std::endl;
            GEngine->renderer->frameScreen->Resize((int)viewportSize.x, (int)viewportSize.y);     //GEngine->renderer->ResizeFramebuffer((int)viewportSize.x, (int)viewportSize.y);
            lastSize = viewportSize;

            // Também redimensiona a projeção da câmera
            GEngine->mainCamera->SetProjection((float)viewportSize.x, (float)viewportSize.y);
            GEngine->mainCamera->SetAspectRatio(viewportSize.x / viewportSize.y);

            GEngine->window->m_width = viewportSize.x;
            GEngine->window->m_height = viewportSize.y;
            // Resize do framebuffer interno, por exemplo usando glViewport e recriando textura
            GEngine->window->SetWindowViewport0();   //glViewport(0, 0, viewportSize.x, viewportSize.y);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            framebufferDirty = true;
        }
        if (!framebufferDirty) {
            // Renderize o framebuffer aqui se houver
            GLuint textureID = GEngine->renderer->GetTextureRendered;
            ImGui::Image((ImTextureID)(uintptr_t)textureID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
        }
        else {
            // opcional: você pode exibir uma tela preta temporária
            ImGui::Dummy(viewportSize);
            framebufferDirty = false; // libera no próximo frame 
        }


        if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize)) {
            if (ImGui::ImageButton("TranslateBtn", iconTranslate, ImVec2(24, 24)))
                currentGizmoMode = ImGuizmo::TRANSLATE;

            ImGui::SameLine();
            if (ImGui::ImageButton("TranslateBtn2", iconRotate, ImVec2(24, 24)))
                currentGizmoMode = ImGuizmo::ROTATE;

            ImGui::SameLine();
            if (ImGui::ImageButton("TranslateBtn3", iconScale, ImVec2(24, 24)))
                currentGizmoMode = ImGuizmo::SCALE;
        }
        ImGui::End();

        // Chama o BeginFrame do ImGuizmo
        ImGuizmo::BeginFrame();

        // ImGuizmo
        Entity selected = scene.GetSelectedEntity();
        if (selected != INVALID_ENTITY) {
            //Presume-se que toda entidade selecionável tem um TransformComponent. 
            // ... código anterior ...
            Transform& tc = scene.GetComponent<Transform>(selected);

            //DESNECESSARIO
            // Atualiza antes do gizmo (garante que a matriz está correta)
            //TransformSystem::UpdateSubtree(scene, selected);

            // Prepara rect / view / proj
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewportSize.x, viewportSize.y);

            glm::mat4 view = GEngine->mainCamera->GetViewMatrix();
            glm::mat4 proj = GEngine->mainCamera->GetProjectionMatrix();

            // IMPORTANTE: aqui usamos a matrix world do transform.
            // Certifique-se que você chamou TransformSystem::UpdateAllTransforms(scene) neste frame antes de desenhar gizmos.
            glm::mat4 worldMat = tc.GetMatrix(); // GetMatrix() retorna model/worldMatrix

            ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
                currentGizmoMode, ImGuizmo::LOCAL, glm::value_ptr(worldMat));

            if (ImGuizmo::IsUsing()) {
                //ApplyGizmoTransform(scene, selected, worldMat); 
                // new world matrix gerada pelo gizmo
                glm::mat4 newWorld = worldMat;

                // se tiver parent -> converte para local
                glm::mat4 parentWorld = glm::mat4(1.0f);
                if (scene.HasComponent<HierarchyComponent>(selected)) {
                    HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(selected);
                    if (h.parent != INVALID_ENTITY && scene.HasComponent<Transform>(h.parent)) {
                        parentWorld = scene.GetComponent<Transform>(h.parent).model;
                        tc.SetLocalFromWorldMatrix(newWorld, parentWorld);
                    }
                    else {
                        tc.SetLocalFromWorldMatrixAsRoot(newWorld);
                    }
                }
                else {
                    tc.SetLocalFromWorldMatrixAsRoot(newWorld);
                }
                 
                // não precisa MarkDirty() porque os SetLocalFromWorldMatrix já fazem isso internamente
                // só garante que o Update do TransformSystem vai recalcular na próxima passada

                // atualiza subtree imediatamente (filhos precisam seguir)
                // não precisa atualizar subtree manual aqui → deixa o sistema cuidar no próximo frame
                // forçar atualização só da subtree editada
                TransformSys::UpdateSubtree(scene, selected);
            }
             
            //if (isSceneHovered && isSceneFocused)     // Manipula o transform da entidade com o gizmo, apenas se a janela de cena estiver focada
        } 

         
        const char* items[] = { "Scene", "Hierarchy", "Inspector", "Project" };
        static int current_item = 0;

        if (ImGui::Begin("Painel Customizável")) {
            if (ImGui::BeginCombo("Tipo de Painel", items[current_item])) {
                for (int i = 0; i < IM_ARRAYSIZE(items); ++i) {
                    bool is_selected = (current_item == i);
                    if (ImGui::Selectable(items[i], is_selected))
                        current_item = i;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            // Renderiza painel baseado na escolha
            /*if (strcmp(items[current_item], "Scene") == 0)
                DrawScene();
            else if (strcmp(items[current_item], "Hierarchy") == 0)
                DrawHierarchy();
            else if (strcmp(items[current_item], "Inspector") == 0)
                DrawInspector();
            else if (strcmp(items[current_item], "Project") == 0)
                DrawProjectFiles();*/
        }
        ImGui::End();

         
        ImGui::End();

    }

    void ApplyGizmoTransform(SceneECS& scene, Entity e, const glm::mat4& newWorld) {
        Transform& t = scene.GetComponent<Transform>(e);
        glm::mat4 parentWorld = glm::mat4(1.0f);
        if (scene.HasComponent<HierarchyComponent>(e)) {
            auto& h = scene.GetComponent<HierarchyComponent>(e);
            if (h.parent != INVALID_ENTITY && scene.HasComponent<Transform>(h.parent))
                parentWorld = scene.GetComponent<Transform>(h.parent).model;
        }
        t.SetLocalFromWorldMatrix(newWorld, parentWorld);
        TransformSys::UpdateSubtree(scene, e);
    }


    const char* GetName() override { return "Scene"; }

private:
    inline static ImVec2 lastSize = { 0, 0 };

    bool framebufferDirty = false;
    ImTextureID iconTranslate = NULL;
    ImTextureID iconRotate = NULL;
    ImTextureID iconScale = NULL;
    ImGuizmo::OPERATION currentGizmoMode = ImGuizmo::TRANSLATE;

    //ImTextureID LoadTexture(const std::string& path);       //LoadIconTexture
};

 
 