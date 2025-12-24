#pragma once
#include "../../Core/EngineContext.h"  
#include "../../Core/InputSystem.h"  
#include "../../Core/Camera.h"  
#include "../../Platform/Window.h"  
#include "../Math/Ray.h"  
#include "../Math/AABB.h"  
#include "../../ECS/Components/ComponentsHelper.h"
#include "../Scene/SceneECS.h"  

//Ele percorre todas entidades com mesh + transform + AABB, testa interseção, e seleciona a mais próxima.
class PickingSystem : public System {
public:  
    void Update(float deltaTime) override { 
        auto& scene = GEngine->scene;
        InputSystem* input = GEngine->input; // Ponte para o InputSystem 
        const unsigned int width = GEngine->window->GetWidth();
        const unsigned int height = GEngine->window->GetHeight();

        //if (ImGuizmo::IsUsing() || ImGuizmo::IsOver())
            //return;

        // Substituindo IsMouseClicked pelo InputSystem
        if (!input->IsMousePressed(MOUSE_BUTTON_LEFT))  return;

        Ray ray = RayFactory::ScreenPosToWorldRay(
            input->GetMousePosition().x,
            input->GetMousePosition().y,
            width, height,
            GEngine->mainCamera->GetViewMatrix(), 
            GEngine->mainCamera->GetProjectionMatrix(), 
            GEngine->mainCamera->GetPosition()
        );

        float closestT = FLT_MAX;
        Entity picked  = INVALID_ENTITY;

        for (Entity e : scene->GetEntitiesWith<MeshRenderer, Transform, AABBComponent>()) {
            auto& mesh = scene->GetComponent<MeshRenderer>(e);
            auto& trans = scene->GetComponent<Transform>(e);
            auto& bound = scene->GetComponent<AABBComponent>(e);

            float t;
            AABB aabb(bound.worldMin, bound.worldMax);

            //std::cout << "IN ANALISE" << std::endl;
            if (aabb.intersects(ray, t)) {
                //std::cout << " NamePIcking " << scene->GetComponent<NameComponent>(e).name << std::endl;
                if (t < closestT) {
                    //std::cout << " Pick " << std::endl;
                    closestT = t;
                    picked = e;
                }
            }
            //std::cout << "out" << std::endl;
        }

        if (picked != INVALID_ENTITY) scene->SelectEntity(picked);
    }
};




















 