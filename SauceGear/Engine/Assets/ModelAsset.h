#pragma once
#include "IAsset.h"
#include "../Resources/Loaders/ModelNode.h" 
#include "../Assets/MeshAsset.h"  
#include "../Core/Log.h"

class ModelAsset : public IAsset {      //vem do arquivo - nunca muda
public:
    std::string name; 

    std::shared_ptr<ModelNode> root;                 // hierarquia
    std::vector<std::shared_ptr<MeshAsset>> meshes;  // geometria

    void Reload() override; 
};




/*

void Reload() override {
        auto fresh = ModelSerializer::Load(path);

        // move apenas o conteúdo
        mesh = std::move(fresh->mesh);

        lastWrite = std::filesystem::last_write_time(path);
    }

*/