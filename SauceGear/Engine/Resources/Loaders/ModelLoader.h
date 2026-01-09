#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../../Assets/ModelAsset.h"
#include "../../Materials/TextureCache.h"
#include "../../Assets/MaterialAsset.h"
#include "../../Materials/PBRMaterial.h"
#include "../../Assets/AssetDatabase.h"
#include "ModelNode.h"


class ModelLoader {
public:
    static std::shared_ptr<ModelAsset> Load(const std::string& path);

private: 
    static std::shared_ptr<MeshAsset> ProcessMesh(
        aiMesh* aiM,
        const aiScene* scene,
        const std::string& dir,
        const std::string& modelName
    );
    
    static std::shared_ptr<MeshAsset> ProcessNodes(
        aiNode* node,
        const aiScene* scene,
        const std::string& dir,
        ModelAsset* modelAsset
    );

    static std::shared_ptr<ModelNode> ProcessNode(aiNode* node);

    static std::shared_ptr<MaterialAsset> CreateMaterialAssetFromAssimp(
        aiMaterial* aiMat,
        const std::string& directory,
        const std::string& modelName
    );
};
