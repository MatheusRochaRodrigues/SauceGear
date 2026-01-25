#pragma once
#include "../../Core/EngineContext.h"  
#include "../../Core/InputSystem.h"  
#include "../../Core/Camera.h"   
#include "../Math/Ray.h"  
#include "../Math/AABB.h"  
#include "../../ECS/Components/TransformComponent.h"
#include "../../ECS/Components/AABBComponent.h"
#include "../Scene/SceneECS.h"  

#include "../ECS/Systems/DebugRenderer.h"

//Warning - EDitor Acomplacao
#include "../Core/EditorState.h"

//Ele percorre todas entidades com mesh + transform + AABB, testa interseção, e seleciona a mais próxima.
class PickingSystem : public System {
public:  
    void Update(float deltaTime) override { 
        auto& scene = GEngine->scene;
        InputSystem* input = GEngine->input; // Ponte para o InputSystem 
        auto& state = *GEngine->editorState;

        //---------- logicMouse() {  
        if (!state.wantsPick) return;  

        Ray ray = RayFactory::ScreenPosToWorldRay(
            state.mouseViewport.x,
            state.mouseViewport.y,
            state.sceneViewportSize.x,
            state.sceneViewportSize.y,
            GEngine->mainCamera->GetViewMatrix(),
            GEngine->mainCamera->GetProjectionMatrix()
        ); 

        //DebugRenderer::Line( ray.origin, ray.at(50), glm::vec3(1, 0, 1), true );

        float closestT = FLT_MAX;
        Entity picked  = INVALID_ENTITY;

        for (Entity e : scene->GetEntitiesWith<TransformComponent, AABBComponent>()) { 
            auto& trans = scene->GetComponent<TransformComponent>(e);
            auto& bound = scene->GetComponent<AABBComponent>(e);

            float t;
            AABB aabb(bound.worldMin, bound.worldMax);
             
            if (aabb.intersect(ray, t)) { 
                if (t < closestT) { 
                    closestT = t;
                    picked = e;
                }
            } 
        } 
        if (picked != INVALID_ENTITY) scene->SelectEntity(picked);

        //Debug
        //Entity e = scene->GetSelectedEntity(); 
        //if (e != INVALID_ENTITY) {
        //    auto aabb = scene->GetComponent<AABBComponent>(e);
        //    DebugRenderer::Cube(
        //        aabb.worldMin,
        //        aabb.worldMax,
        //        glm::vec3(1,1,0),
        //        true   // Unity-style: redesenha todo frame
        //    );
        //}
    }
     
};




















 