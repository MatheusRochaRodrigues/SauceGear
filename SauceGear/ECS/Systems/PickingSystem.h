#pragma once
#include "../../Core/EngineContext.h"  
#include "../../Core/InputSystem.h"  
#include "../../Core/Camera.h"  
#include "../../Platform/Window.h"  
#include "../Utils/utilsAABB.h"  
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
        // Substituindo IsMouseClicked pelo InputSystem
        if (!input->IsMousePressed(MOUSE_BUTTON_LEFT)) 
            return;

        Ray ray = utilsAABB::ScreenPosToWorldRay(
            input->GetMousePosition().x,
            input->GetMousePosition().y,
            width, height,
            GEngine->mainCamera->GetViewMatrix(), 
            GEngine->mainCamera->GetProjectionMatrix(), 
            GEngine->mainCamera->GetPosition()
        );

        float closestT = FLT_MAX;
        Entity picked = INVALID_ENTITY;

        for (Entity e : scene->GetEntitiesWith<MeshRenderer, Transform, AABBComponent>()) {
            auto& mesh = scene->GetComponent<MeshRenderer>(e);
            auto& trans = scene->GetComponent<Transform>(e);
            auto& aabb = scene->GetComponent<AABBComponent>(e);

            float t;
            if (utilsAABB::IntersectRayAABB(ray, aabb, t)) {
                if (t < closestT) {
                    closestT = t;
                    picked = e;
                }
            }
        }

        if (picked != INVALID_ENTITY)
            scene->SelectEntity(picked);
    }
};
























//void Update(float deltaTime) override {
//    if (!IsMouseClicked()) return;
//
//    Ray ray = ScreenPosToWorldRay(
//        GetMouseX(), GetMouseY(),
//        GetScreenWidth(), GetScreenHeight(),
//        view, projection, cameraPos
//    );
//
//    float closestT = FLT_MAX;
//    Entity picked = INVALID_ENTITY;
//
//    for (Entity e : scene->GetEntitiesWith<MeshComponent, Transform>()) {
//        auto& mesh = scene->GetComponent<MeshComponent>(e);
//        auto& trans = scene->GetComponent<Transform>(e);
//        auto& aabb = scene->GetComponent<AABBComponent>(e);
//
//        float t;
//        if (IntersectRayAABB(ray, aabb, t)) {
//            if (t < closestT) {
//                closestT = t;
//                picked = e;
//            }
//        }
//    }
//
//    if (picked != INVALID_ENTITY)
//        scene->SelectEntity(picked);
//}