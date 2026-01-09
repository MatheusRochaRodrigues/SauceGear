#pragma once
#include "../System.h"
#include "../Scene/SceneECS.h"
#include "../Core/EngineContext.h"
#include "../Math/AABB.h"
#include "../Components/AABBComponent.h"
#include "../Utils/AABBBuilder.h"
#include "../Components/TransformComponent.h" 

class AABBSystem : public System {
public:
    void Update(float dt) override {
        SceneECS& scene = *GEngine->scene;
         
        for (Entity e : scene.GetEntitiesWith<AABBComponent, MeshRenderer, TransformComponent>()) {  
            TransformComponent& t    = scene.GetComponent<TransformComponent>(e);
            AABBComponent&      aabb = scene.GetComponent<AABBComponent>(e);
            MeshRenderer&       mr   = scene.GetComponent<MeshRenderer>(e);

            if (aabb.dirtyLocal) {
                if (!mr.mesh || mr.mesh->mesh->vertices.empty()) continue; //std::cout << "errado aq - AABB_SYSTEM " << std::endl; continue;
                  
                auto local = AABBBuilder::ComputeLocalAABB(*mr.mesh->mesh);
                aabb.setLocal(local.min, local.max);
            }

            // Se o transform acabou de ser atualizado, o AABB precisa atualizar
            if (t.transformChangedThisFrame || aabb.dirty) {
                //mais rapido
                //aabb.world = TransformAABB(aabb.local, t.model);

                //mais robuzto e pesado,use se usar coisas como shear, validando bounds e etc
                UpdateWorldAABB(aabb, t.model);
                aabb.dirty = false;
            }

        }  
    }

private:

/* We are generating a box in the space defined by the minimum and maximum values ​​of AABB.
          6──────7
         /│     /│
        4──────5 │
        │ │    │ │
        │ 2────│─3
        │/     │/
        0──────1 
*/

    static void UpdateWorldAABB(AABBComponent& aabb, const glm::mat4& M) {
        // transformar os 8 cantos do AABB local
        glm::vec3 corners[8] = {
            {aabb.localMin.x, aabb.localMin.y, aabb.localMin.z},
            {aabb.localMax.x, aabb.localMin.y, aabb.localMin.z},
            {aabb.localMin.x, aabb.localMax.y, aabb.localMin.z},
            {aabb.localMax.x, aabb.localMax.y, aabb.localMin.z},
            {aabb.localMin.x, aabb.localMin.y, aabb.localMax.z},
            {aabb.localMax.x, aabb.localMin.y, aabb.localMax.z},
            {aabb.localMin.x, aabb.localMax.y, aabb.localMax.z},
            {aabb.localMax.x, aabb.localMax.y, aabb.localMax.z},
        };

        glm::vec3 minW(FLT_MAX);
        glm::vec3 maxW(-FLT_MAX);

        for (int i = 0; i < 8; ++i) {
            glm::vec3 p = glm::vec3(M * glm::vec4(corners[i], 1.0f));
            minW = glm::min(minW, p);
            maxW = glm::max(maxW, p);
        }

        aabb.worldMin = minW;
        aabb.worldMax = maxW;
    }
};
