#pragma once
#include "../Assets/ModelAsset.h" 

//class ModelInstance {       ///instância viva na cena
//public:
    /*std::shared_ptr<ModelNodeInstance> root;

    explicit ModelInstance(const std::shared_ptr<ModelAsset>& asset) {
        root = Build(asset->root);
    }

private:
    static std::shared_ptr<ModelNodeInstance> Build(const std::shared_ptr<ModelNode>& node) {
        auto inst = std::make_shared<ModelNodeInstance>();

        if (node->mesh) {
            inst->mesh = std::make_shared<MeshInstance>(node->mesh);

            for (auto& sm : node->mesh->submeshes)
                inst->materials.push_back(
                    std::make_shared<MaterialInstance>(sm.materialAsset)
                );

        }

        for (auto& c : node->children)
            inst->children.push_back(Build(c));

        return inst;
    }*/
//};





/*
permite:

múltiplas instâncias do mesmo modelo

cada uma com transform próprio

cada uma com material override

-> Sem isso você DUPLICARIA asset, o que é errado.
*/
