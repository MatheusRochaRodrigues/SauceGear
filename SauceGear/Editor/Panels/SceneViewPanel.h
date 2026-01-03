#pragma once
#include "IPanel.h"
#include "../ECS/Components/ComponentsHelper.h" 
#include "../Graphics/Renderer.h"  
#include "Load.h"

#include "../../ECS/Systems/TransformSystem.h" 

#include "../Core/EditorState.h"
#include "../../Core/InputSystem.h"  

struct SceneViewPanel : IPanel { 

    SceneViewPanel() { 
        iconTranslate = guiLoadTexture("assets/icons/Translate.png");
        iconRotate    = guiLoadTexture("assets/icons/Rotate.png");
        iconScale     = guiLoadTexture("assets/icons/Scale.png");
    } 

    void Draw(SceneECS& scene) override {
        ImGui::Begin("Scene");

        EditorState& state = *GEngine->editorState;
        InputSystem* input = GEngine->input;

        // --------------------------------------------------
        // 1. RESET DE INTENÇÕES
        // --------------------------------------------------
        cleanState();

        // --------------------------------------------------
        // 2. VIEWPORT SIZE / FRAMEBUFFER
        // --------------------------------------------------

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

        // --------------------------------------------------
    // 3. ATUALIZA VIEWPORT STATE (ANTES DO GIZMO)
    // --------------------------------------------------
        //GetWindowPos() retorna o canto superior esquerdo de toda janela imgui aberta e nao da atual janela 
        state.sceneViewportPos = {
            ImGui::GetWindowPos().x,
            ImGui::GetWindowPos().y
        };

        state.sceneViewportSize = {
            viewportSize.x,
            viewportSize.y
        };

        state.sceneViewportHovered = ImGui::IsWindowHovered();
        state.sceneViewportFocused = ImGui::IsWindowFocused();

        glm::vec2 mouse = input->GetMousePosition();
        state.mouseScreen = mouse;
        state.mouseViewport.x = mouse.x - state.sceneViewportPos.x;
        // without flip Y
        state.mouseViewport.y = mouse.y - state.sceneViewportPos.y; // flip de Y para IMGUI senao seria apenas -> mouse.y - state.sceneViewportPos.y
        // with flip y
        //state.mouseViewport.y = state.sceneViewportSize.y - (mouse.y - state.sceneViewportPos.y);
         
        // 4. TOOLBAR 
        DrawToolbar();
         
        // 5. GIZMO 

        // Chama o BeginFrame do ImGuizmo
        ImGuizmo::BeginFrame();

        // ImGuizmo
        Entity selected = scene.GetSelectedEntity();
        if (selected != INVALID_ENTITY) {
            //Presume-se que toda entidade selecionável tem um TransformComponent. 
            // ... código anterior ...
            TransformComponent& tc = scene.GetComponent<TransformComponent>(selected);

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
            glm::mat4 objectWorld = tc.GetMatrix();
            glm::mat4 pivotWorld = objectWorld;

            bool usingAabbPivot =
                scene.HasComponent<AABBComponent>(selected) &&
                currentGizmoSpace == ImGuizmo::LOCAL;

            if (usingAabbPivot) {
                AABBComponent& aabb = scene.GetComponent<AABBComponent>(selected);
                // centro LOCAL da malha
                glm::vec3 localCenter = (aabb.localMin + aabb.localMax) * 0.5f;
                // matriz do pivot (world)
                pivotWorld = objectWorld * glm::translate(glm::mat4(1.0f), localCenter);
            }
            // gizmo começa no pivot
            glm::mat4 gizmoWorld = pivotWorld;

            ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
                currentGizmoMode, currentGizmoSpace, glm::value_ptr(gizmoWorld)); //LOCAL


            // --------------------------------------------------
            // 6. ESTADO DO GIZMO + PICK
            // --------------------------------------------------
            state.gizmoUsing = ImGuizmo::IsUsing();
            state.gizmoOver = ImGuizmo::IsOver();

            state.wantsPick =
                state.sceneViewportHovered &&
                !state.gizmoUsing &&
                !state.gizmoOver &&
                input->IsMousePressed(MOUSE_BUTTON_LEFT);


            if (ImGuizmo::IsUsing()) {
                // delta em espaço WORLD
                glm::mat4 delta = glm::inverse(pivotWorld) * gizmoWorld;
                glm::mat4 newWorld = objectWorld * delta;
                glm::mat4 parentWorld(1.0f);

                if (scene.HasComponent<HierarchyComponent>(selected)) {
                    HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(selected);
                    if (h.parent != INVALID_ENTITY &&
                        scene.HasComponent<TransformComponent>(h.parent)) {
                        parentWorld = scene.GetComponent<TransformComponent>(h.parent).model;
                    }
                }

                // CONVERTE WORLD -> LOCAL (Unity-like)
                tc.SetWorldPosition(glm::vec3(newWorld[3]), parentWorld);

                // rotação
                glm::quat newRot;
                glm::vec3 skew, pos, scl;
                glm::vec4 persp;
                glm::decompose(newWorld, scl, newRot, pos, skew, persp);
                tc.SetWorldRotation(glm::normalize(newRot), parentWorld);

                // escala
                tc.SetWorldScale(scl, parentWorld);

                // Atualiza só a subtree editada
                TransformSys::UpdateSubtree(scene, selected);
            }
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

    void DrawToolbar() {
        if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize)) {
            //Orientations
            if (ImGui::ImageButton("TranslateBtn", iconTranslate, ImVec2(24, 24)))
                currentGizmoMode = ImGuizmo::TRANSLATE;

            ImGui::SameLine();
            if (ImGui::ImageButton("TranslateBtn2", iconRotate, ImVec2(24, 24)))
                currentGizmoMode = ImGuizmo::ROTATE;

            ImGui::SameLine();
            if (ImGui::ImageButton("TranslateBtn3", iconScale, ImVec2(24, 24)))
                currentGizmoMode = ImGuizmo::SCALE;

            //Space
            ImGui::SameLine();
            const char* spaceLabel = (currentGizmoSpace == ImGuizmo::WORLD) ? "WORLD" : "LOCAL";

            if (ImGui::Button(spaceLabel, ImVec2(60, 24))) {
                currentGizmoSpace =
                    (currentGizmoSpace == ImGuizmo::WORLD)
                    ? ImGuizmo::LOCAL
                    : ImGuizmo::WORLD;
            }

            ImGui::SameLine();
            ImGui::Checkbox("Snap", &useSnap);

            if (useSnap) {
                if (currentGizmoMode == ImGuizmo::TRANSLATE)
                    snapValues[0] = snapTranslate;
                else if (currentGizmoMode == ImGuizmo::ROTATE)
                    snapValues[0] = snapRotate;
                else if (currentGizmoMode == ImGuizmo::SCALE)
                    snapValues[0] = snapScale;
            }


        }
        ImGui::End();
    } 
     
    const char* GetName() override { return "Scene"; }

    void cleanState() {
        EditorState& state = *GEngine->editorState;
        state.wantsPick = true; //false
        state.gizmoUsing = false;
        state.gizmoOver = false; 
    } 

private:
    inline static ImVec2 lastSize = { 0, 0 };

    bool framebufferDirty = false;
    ImTextureID iconTranslate = NULL;
    ImTextureID iconRotate = NULL;
    ImTextureID iconScale = NULL;
    ImGuizmo::OPERATION currentGizmoMode = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE currentGizmoSpace = ImGuizmo::WORLD;

    //snap list
    bool useSnap = false;
    float snapTranslate = 0.5f;
    float snapRotate = 15.0f;
    float snapScale = 0.1f; 
    float snapValues[3];

    //ImTextureID LoadTexture(const std::string& path);       //LoadIconTexture
};

 
 





//void saveState() {
//    auto state = GEngine->editorState;
//
//    state->gizmoUsing = ImGuizmo::IsUsing();
//    state->gizmoOver = ImGuizmo::IsOver();
//
//    state->sceneViewportPos = {
//        ImGui::GetWindowPos().x,
//        ImGui::GetWindowPos().y
//    };
//
//    state->sceneViewportSize = {
//        ImGui::GetContentRegionAvail().x,
//        ImGui::GetContentRegionAvail().y
//    };
//
//    state->sceneViewportHovered = ImGui::IsWindowHovered();
//    state->sceneViewportFocused = ImGui::IsWindowFocused();
//
//
//    InputSystem* input = GEngine->input; // Ponte para o InputSystem   
//    state->wantsPick = state->sceneViewportHovered && !state->gizmoUsing && !state->gizmoOver
//        && input->IsMousePressed(MOUSE_BUTTON_LEFT);
//
//
//    //Legacy de picking input sobre oc odigo acima 
//    /*
//    if (ImGuizmo::IsUsing() || ImGuizmo::IsOver()) return;
//    // Substituindo IsMouseClicked pelo InputSystem
//    if (!input->IsMousePressed(MOUSE_BUTTON_LEFT))  return;
//    */
//
//    glm::vec2 mouse = input->GetMousePosition();
//    state->mouseScreen = mouse;
//    state->mouseViewport = {
//        mouse.x - state->sceneViewportPos.x,
//        state->sceneViewportSize.y - (mouse.y - state->sceneViewportPos.y)
//    };
//
//
//
//    //alternativamente para picking
//    //sendo uma forma mais explicita de descrever oq acontece em mouseViewport
//
//    /*
//    glm::vec2 sceneViewportPos = state->sceneViewportPos;
//    glm::vec2 sceneViewportSize = state->sceneViewportSize;
//
//
//    glm::vec2 mouse = input->GetMousePosition();
//
//    float localX = mouse.x - sceneViewportPos.x;
//    float localY = mouse.y - sceneViewportPos.y;
//
//    // fora da viewport → não picka
//    if (localX < 0 || localY < 0 ||
//        localX > sceneViewportSize.x ||
//        localY > sceneViewportSize.y)
//        return;
//
//    // inverte Y (ImGui vs OpenGL)
//    localY = sceneViewportSize.y - localY;
//    */
//}