#pragma once
#include <unordered_map>
#include <vector>
#include "../../Instancing/MaterialInstance.h"
#include "../../Instancing/MeshInstance.h"
#include "../../ECS/Components/MeshComponent.h" 
#include "../../Materials/MaterialBinder.h" 

//struct DrawBatch {
//    MaterialBase* base;
//    std::vector<Mesh*> meshes;
//}; 

enum class RenderPassType {
    Geometry,
    DeferredLight,
    Forward,
    Shadow
};

// MELHORAS, USE MODEL ISNTANCE PARA ORGANIZAR, FAZER BATCHING E MUITAS COISAS INCLUSIVE HIERARQUICA ENTRE AS MALHAS PARA EVITAR
//  DRAW CALLS ABSURDOS E MINIMIZAR
struct MeshRenderer {
    std::shared_ptr<MeshInstance> mesh; 
    std::vector<std::shared_ptr<MaterialInstance>> materials;

    void Draw() {   //RenderPassType pass
        auto* base = materials[0]->asset->base.get();

        // if (!PipelineAccepts(base->domain, pass)) return;

        base->shader->use();

        MaterialBinder::Bind(
            *materials[0],
            *materials[0]->asset,
            *base
        );

        mesh->Draw(); // desenha todos submeshes
    }
};



/*
void BuildFromMesh() {
    materials.clear();
    materials.reserve(mesh->submeshes.size());

    for (auto& sm : mesh->submeshes) {
        materials.push_back(sm.materialAsset->Instantiate());
    }
}
*/