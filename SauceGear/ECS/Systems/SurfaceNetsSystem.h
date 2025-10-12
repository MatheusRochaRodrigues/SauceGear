#pragma once
#include "../ECS/System.h"
#include "../Scene/SceneECS.h"
#include "../Components/SurfaceNetsComponent.h"
#include "../Components/MeshRenderer.h"

class SurfaceNetsSystem : public System {
public:
    void Update(float dt) override {
        // process entities with SurfaceNetsComponent
        //auto ents = GEngine->scene->GetEntitiesWith<SurfaceNetsComponent>();
        //for (auto e : ents) {
        //    auto& comp = GEngine->scene->GetComponent<SurfaceNetsComponent>(e);
        //    if (!comp.dirty) continue;

        //    // CPU path for now
        //    if (comp.chunk->mesh) {
        //        delete comp.chunk->mesh; comp.chunk->mesh = nullptr;
        //    }
        //    comp.chunk->mesh = SurfaceNetsCPU::Generate(comp.grid, comp.params);

        //    // attach/ensure MeshRenderer exists and set mesh
        //    if (!GEngine->scene->HasComponent<MeshRenderer>(e)) {
        //        auto& mr = GEngine->scene->AddComponent<MeshRenderer>(e, comp.mesh);
        //        mr.RebuildBatches();
        //    }
        //    else {
        //        auto& mr = GEngine->scene->GetComponent<MeshRenderer>(e);
        //        mr.SetMesh(comp.mesh); // implemente SetMesh no seu MeshRenderer
        //        mr.RebuildBatches();
        //    }

        //    comp.dirty = false;
        }
    }
};
