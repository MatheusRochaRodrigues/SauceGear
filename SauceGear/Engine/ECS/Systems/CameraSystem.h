#pragma once
#include "../../Core/Camera.h" 
#include "../../ECS/System.h" 
#include "../../ECS/Components/CameraComponent.h"
#include "../../ECS/Components/TransformComponent.h" 
#include "../../Core/Input.h"
#include "../../Core/EditorState.h"
#include <imgui.h>

class CameraSystem : public System {
public:
    void Update(float deltaTime) override {
        try {
            auto entities = GEngine->scene->GetEntitiesWith<CameraComponent, TransformComponent>();

            Entity mainCameraEntity = INVALID_ENTITY;

            for (Entity e : entities) {
                auto& camComp = GEngine->scene->GetComponent<CameraComponent>(e);
                auto& trans =   GEngine->scene->GetComponent<TransformComponent>(e);

                if (!camComp.camera) {
                    std::cerr << "[ERRO] CameraComponent com ponteiro nulo\n";
                    continue;
                }

                Camera* cam = camComp.camera;  
                // Movimento 
                bool sprint = Input::GetKey(KEY_LEFT_SHIFT); 
                // Movimentos principais
                if (Input::GetKey(KEY_W)) cam->ProcessKeyboard(FORWARD, deltaTime, sprint);
                if (Input::GetKey(KEY_S)) cam->ProcessKeyboard(BACKWARD, deltaTime, sprint);
                if (Input::GetKey(KEY_A)) cam->ProcessKeyboard(LEFT, deltaTime, sprint);
                if (Input::GetKey(KEY_D)) cam->ProcessKeyboard(RIGHT, deltaTime, sprint);

                if (Input::GetKey(KEY_LEFT_CONTROL)) cam->ProcessKeyboard(DOWN, deltaTime, sprint);
                if (Input::GetKey(KEY_SPACE)) cam->ProcessKeyboard(UP, deltaTime, sprint);



                GLFWwindow* win = GEngine->window->GetNativeWindow();
                if (Input::GetMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

                    Input::ClearMouseDelta();
                    mouseLocked = true;
                }

                if (Input::GetMouseButtonUp(MOUSE_BUTTON_RIGHT)) {
                    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);

                    Input::ClearMouseDelta();
                    mouseLocked = false;
                }

                if (mouseLocked)
                    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
                else
                    ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
                 
                EditorState& state = *GEngine->editorState; 
                if (mouseLocked /*&& state.sceneViewportHovered*/ && !state.gizmoUsing) {     // mouseLocked == Input::GetMouseButton(MOUSE_BUTTON_RIGHT)          !ImGui::GetIO().WantCaptureMouse
                    glm::vec2 delta = Input::GetMouseDelta();   
                    cam->ProcessMouseMovement(delta.x, -delta.y); // inverte y
                }
                 

                cam->UpdtMatrices(); 
                if (camComp.isMain) mainCameraEntity = e;
            }

            if (mainCameraEntity != INVALID_ENTITY) {
                auto& mainCam = GEngine->scene->GetComponent<CameraComponent>(mainCameraEntity);
                GEngine->SetActiveCamera(mainCam.camera); //GEngine->renderer->SetActiveCamera(mainCam.camera);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[EXCEÇÃO - CameraSystem] " << e.what() << "\n";
        }
    }
private:
    bool mouseLocked = false; 

    int currentIndex = 0;
};


/*
   Flags para definir movimento bruto do mause -> raw, sem monitor ou isstema opercional intervindo nessa operação
                    if (glfwRawMouseMotionSupported()) glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

                    if (glfwRawMouseMotionSupported()) glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);



*/


/*

                if (Input::GetMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                    glfwSetInputMode(
                        GEngine->window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED
                    );
                    GEngine->input->ResetMouseDelta(); // você pode implementar isso
                    mouseLocked = true;

                    // Diga pro ImGui não capturar o mouse
                    ImGui::GetIO().WantCaptureMouse = true; // bloqueia GUI temporariamente
                }

                if (Input::GetMouseButtonUp(MOUSE_BUTTON_RIGHT) ) { // || ImGui::GetIO().WantCaptureMouse
                    glfwSetInputMode(
                        GEngine->window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL
                    );
                    mouseLocked = false;

                    // Libera GUI novamente
                    ImGui::GetIO().WantCaptureMouse = false;
                }

                //// Rotação com o mouse (Ex: se botão direito pressionado)
                if (mouseLocked) {     // mouseLocked == Input::GetMouseButton(MOUSE_BUTTON_RIGHT)
                    glm::vec2 delta = Input::GetMouseDelta();
                    cam->ProcessMouseMovement(delta.x, -delta.y); // inverte y
                }
















         if (Input::GetMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                    GLFWwindow* win = GEngine->window->GetNativeWindow();

                    int w = (int)GEngine->renderer->width;
                    int h = (int)GEngine->renderer->height;

                    glfwGetWindowSize(win, &w, &h);

                    // CENTRALIZA
                    glfwSetCursorPos(win, w * 0.5, h * 0.5);

                    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                    // ZERA DELTA CORRETAMENTE
                    GEngine->input->ResetMouseDeltaTo(w * 0.5, h * 0.5);

                    mouseLocked = true;
                }

                //if (Input::GetMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                //    glfwSetInputMode(
                //        GEngine->window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED
                //    );
                //    GEngine->input->ResetMouseDelta(); // você pode implementar isso
                //    mouseLocked = true;
                //}

                if (Input::GetMouseButtonUp(MOUSE_BUTTON_RIGHT)) {
                    glfwSetInputMode(
                        GEngine->window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL
                    );
                    mouseLocked = false;
                }

                //// Rotação com o mouse (Ex: se botão direito pressionado)
                if (mouseLocked && !ImGui::GetIO().WantCaptureMouse) {     // mouseLocked == Input::GetMouseButton(MOUSE_BUTTON_RIGHT)
                    glm::vec2 delta = Input::GetMouseDelta();
                    cam->ProcessMouseMovement(delta.x, -delta.y); // inverte y
                }







                //// Rotação com o mouse (Ex: se botão direito pressionado)
                // -------- Mouse Lock / Unlock --------
                GLFWwindow* win = GEngine->window->GetNativeWindow();
                static glm::vec2 lastMousePos = glm::vec2(0.0f);
                int w, h;
                glfwGetWindowSize(win, &w, &h);
                glm::vec2 windowCenter = glm::vec2(w * 0.5f, h * 0.5f);

                if (Input::GetMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                    glfwSetCursorPos(win, windowCenter.x, windowCenter.y);
                    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    lastMousePos = windowCenter;
                    mouseLocked = true;
                }

                if (Input::GetMouseButtonUp(MOUSE_BUTTON_RIGHT)) {
                    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    mouseLocked = false;
                }

                // -------- Rotação com o Mouse --------
                if (mouseLocked && !ImGui::GetIO().WantCaptureMouse) {
                    glm::dvec2 currentPos;
                    glfwGetCursorPos(win, &currentPos.x, &currentPos.y);
                    glm::vec2 delta = glm::vec2(currentPos) - lastMousePos;
                    lastMousePos = glm::vec2(currentPos);

                    // Gira a câmera
                    cam->ProcessMouseMovement(delta.x, -delta.y);

                    // Opcional: recentraliza o cursor para evitar overflow
                    glfwSetCursorPos(win, windowCenter.x, windowCenter.y);
                    lastMousePos = windowCenter;
                }



*/


/*


void Update(float deltaTime) override {
        try {
            auto entities = GEngine->scene->GetEntitiesWith<CameraComponent, TransformComponent>();

            Entity mainCameraEntity = INVALID_ENTITY;

            for (Entity e : entities) {
                auto& camComp = GEngine->scene->GetComponent<CameraComponent>(e);
                auto& trans =   GEngine->scene->GetComponent<TransformComponent>(e);

                if (!camComp.camera) {
                    std::cerr << "[ERRO] CameraComponent com ponteiro nulo\n";
                    continue;
                }

                Camera* cam = camComp.camera;
                // Movimento
                bool sprint = Input::GetKey(KEY_LEFT_SHIFT);
                // Movimentos principais
                if (Input::GetKey(KEY_W)) cam->ProcessKeyboard(FORWARD, deltaTime, sprint);
                if (Input::GetKey(KEY_S)) cam->ProcessKeyboard(BACKWARD, deltaTime, sprint);
                if (Input::GetKey(KEY_A)) cam->ProcessKeyboard(LEFT, deltaTime, sprint);
                if (Input::GetKey(KEY_D)) cam->ProcessKeyboard(RIGHT, deltaTime, sprint);

                if (Input::GetKey(KEY_LEFT_CONTROL)) cam->ProcessKeyboard(DOWN, deltaTime, sprint);
                if (Input::GetKey(KEY_SPACE)) cam->ProcessKeyboard(UP, deltaTime, sprint);

                //// Rotação com o mouse (Ex: se botão direito pressionado)
                if (Input::GetMouseButton(MOUSE_BUTTON_RIGHT)) {
                    glm::vec2 delta = Input::GetMouseDelta();
                    cam->ProcessMouseMovement(delta.x, -delta.y); // inverte y
                }
                cam->UpdtMatrices();

                if (camComp.isMain) mainCameraEntity = e;
            }

            if (mainCameraEntity != INVALID_ENTITY) {
                auto& mainCam = GEngine->scene->GetComponent<CameraComponent>(mainCameraEntity);
                GEngine->SetActiveCamera(mainCam.camera); //GEngine->renderer->SetActiveCamera(mainCam.camera);
            }
        }


*/