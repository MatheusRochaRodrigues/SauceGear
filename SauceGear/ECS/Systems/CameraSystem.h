#pragma once
#include "../../Core/Camera.h" 
#include "../../ECS/Components/ComponentsHelper.h"
#include "../../Core/Input.h"

class CameraSystem : public System {
public:
    void Update(float deltaTime) override {
        try {
            auto entities = GEngine->scene->GetEntitiesWith<CameraComponent, Transform>();

            Entity mainCameraEntity = INVALID_ENTITY;

            for (Entity e : entities) {
                auto& camComp = GEngine->scene->GetComponent<CameraComponent>(e);
                auto& trans =   GEngine->scene->GetComponent<Transform>(e);

                if (!camComp.camera) {
                    std::cerr << "[ERRO] CameraComponent com ponteiro nulo\n";
                    continue;
                }

                Camera* cam = camComp.camera;

                //camComp.camera->Position = trans.position;
                //camComp.camera->Yaw      = trans.rotation.y;
                //camComp.camera->Pitch    = trans.rotation.x;
                //camComp.camera->ProcessMouseMovement(0, 0); // atualiza vetores

                // Movimento
                if (Input::GetKey(KEY_W)) cam->ProcessKeyboard(FORWARD,  deltaTime);
                if (Input::GetKey(KEY_S)) cam->ProcessKeyboard(BACKWARD, deltaTime);
                if (Input::GetKey(KEY_A)) cam->ProcessKeyboard(LEFT,     deltaTime);
                if (Input::GetKey(KEY_D)) cam->ProcessKeyboard(RIGHT,    deltaTime);

                //// Rotação com o mouse (Ex: se botão direito pressionado)
                if (Input::GetMouseButton(MOUSE_BUTTON_RIGHT)) {
                    glm::vec2 delta = Input::GetMouseDelta();
                    cam->ProcessMouseMovement(delta.x, -delta.y); // inverte y
                }

                //// Atualiza posição no transform também
                /*trans.position =   cam->Position;
                trans.rotation.y = cam->Yaw;
                trans.rotation.x = cam->Pitch; */
                

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
    int currentIndex = 0;
};
