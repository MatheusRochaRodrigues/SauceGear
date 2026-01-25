#pragma once
#include <vector>
#include <memory>
#include "../../Instancing/MaterialInstance.h"
#include "../../Instancing/MeshInstance.h"
#include "../../Materials/MaterialBinder.h"
#include "../Reflection/Macros.h"

enum class RenderPassType {
    Geometry,
    DeferredLight,
    Forward,
    Shadow
};

struct DrawBatch {
    std::shared_ptr<MaterialInstance> material;
    std::vector<uint32_t> submeshes;
};

struct MeshRenderer {
    std::shared_ptr<MeshInstance> mesh;
    std::vector<std::shared_ptr<MaterialInstance>> materials; // indexado por materialIndex    std::unordered_map<std::shared_ptr<MaterialInstance>, std::vector<SubMesh*>> batches;
    std::vector<DrawBatch> batches;


    REFLECT_CLASS(MeshRenderer) {
        REFLECT_HEADER("MeshRenderer");  

        REFLECT_ADD_COMPONENT();
    }

    void BuildBatches() {
        batches.clear();
        if (!mesh) return;

        std::unordered_map<uint32_t, uint32_t> lut;

        for (uint32_t i = 0; i < mesh->mesh->submeshes.size(); i++) {
            const auto& sm = mesh->mesh->submeshes[i];
            uint32_t matIndex = sm.indexMaterialAsset;

            if (matIndex >= materials.size()) continue;
            auto& mat = materials[matIndex];
            if (!mat) continue;

            auto it = lut.find(matIndex);
            if (it == lut.end()) {
                uint32_t batchIndex = static_cast<uint32_t>(batches.size());
                lut[matIndex] = batchIndex;
                batches.push_back({ mat, { i } });
            }
            else {
                batches[it->second].submeshes.push_back(i);
            }
        }
    }

    void Draw() {
        if (!mesh) return;

        for (auto& batch : batches) {
            auto* base = batch.material->asset->base.get();

            base->shader->use();
            MaterialBinder::Bind(
                *batch.material,
                *batch.material->asset,
                *base
            );

            for (uint32_t sm : batch.submeshes)
                mesh->DrawSubmesh(sm);
        }
    }

    void Draw(Shader* shader) {
        if (!mesh) return;

        for (auto& batch : batches) {
            auto* base = batch.material->asset->base.get();
             
            MaterialBinder::Bind(
                *batch.material,
                *batch.material->asset,
                *base
            );

            for (uint32_t sm : batch.submeshes)
                mesh->DrawSubmesh(sm);
        }
    }

    void DrawMesh() {
        if (!mesh) return; 
        for (auto& batch : batches)  
            for (uint32_t sm : batch.submeshes)
                mesh->DrawSubmesh(sm);  
    }
};



// if (!PipelineAccepts(base->domain, pass)) return;            //RenderPassType pass
 


/*
 
struct Entry {
    std::shared_ptr<MeshInstance> mesh;
    uint32_t index = 0;                                         //std::vector<std::shared_ptr<MaterialInstance>> materials; // 1 por submesh
};

*/