#pragma once

#include "../../Core/EngineContext.h"  
#include "../SceneECS.h"  
#include "../System.h"
#include "../Components/AABBComponent.h"
#include "../Components/Transform.h"
#include "../Components/MeshRenderer.h"
#include "../utilsAABB.h"

class BoundsSystem : public System {
public:  
    void Update(float dt) override {
        auto& scene = GEngine->scene;
        for (Entity e : scene->GetEntitiesWith<MeshRenderer, Transform, AABBComponent>()) {
            auto& transform = scene->GetComponent<Transform>(e);

            if (!transform.dirty) continue; // só recalcula se o transform mudou

            auto& mesh = scene->GetComponent<MeshRenderer>(e).mesh;
            auto& aabb = scene->GetComponent<AABBComponent>(e); 

            aabb = utilsAABB::CalculateAABB(mesh, transform.GetMatrix());        //transform.matrix
            //sphere = utilsAABB::CalculateBoundingSphere(mesh, transform.matrix);

            transform.dirty = false; // marca como atualizado
        }
    }

};
