#include "ModelSerializer.h"
#include <nlohmann/json.hpp>
#include <fstream>

#include "../Assets/AssetDatabase.h"
#include "../Assets/MaterialAsset.h"
#include "../Resources/Loaders/ModelLoader.h"
#include "../Assets/AssetLoader.h"

using json = nlohmann::json;

std::shared_ptr<ModelAsset> ModelSerializer::Load(const std::string& path) {
    json j = json::parse(std::ifstream(path));

    auto asset = std::make_shared<ModelAsset>();
    asset->path = path;
    asset->lastWrite = std::filesystem::last_write_time(path);

    // caminho do modelo (fbx, gltf, etc)
    std::string meshPath = j["mesh"];

    // carrega estrutura do modelo (SEM GPU)
    auto loaded = ModelLoader::Load(meshPath);

    asset->root = loaded->root;
    asset->meshes = std::move(loaded->meshes);
    asset->name = std::filesystem::path(meshPath).stem().string();

    // override de materiais por nome de submesh
    if (j.contains("materials")) {
        for (auto& mesh : asset->meshes) {
            for (auto& sm : mesh->submeshes) {
                if (j["materials"].contains(sm.name)) {
                    std::string matPath = j["materials"][sm.name];
                    sm.materialAsset = LoadAsset<MaterialAsset>(matPath);
                }
            }
        }
    }

    return asset;
}







/*
std::shared_ptr<ModelAsset> ModelSerializer::Load(const std::string& path) {
    json j;
    std::ifstream(path) >> j;

    auto asset = std::make_shared<ModelAsset>();
    asset->path = path;
    asset->lastWrite = std::filesystem::last_write_time(path);

    // 1️ carrega FBX
    asset->mesh.reset(ModelLoader::LoadModel(j["mesh"]));

    // 2️ aplica materiais por nome de SubMesh
    for (auto& sm : asset->mesh->submeshes) {
        if (j["materials"].contains(sm.name)) {
            std::string matPath = j["materials"][sm.name];

            sm.materialAsset = AssetDatabase::Load<MaterialAsset>(
                matPath,
                [&] { return MaterialSerializer::Load(matPath); }
            );
        }
    }

    return asset;
}
*/