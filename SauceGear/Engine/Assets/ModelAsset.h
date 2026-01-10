#pragma once
#include "IAsset.h"
#include "../Resources/Loaders/HierarchyNode.h" 
#include "../Assets/MeshAsset.h"  
#include "../Core/Log.h"

class ModelAsset : public IAsset {      //vem do arquivo - nunca muda
public:
    std::string name; 

    std::shared_ptr<HierarchyNode> root;                 // hierarquia
    std::vector<std::shared_ptr<MeshAsset>> meshes;  // geometria

    // materiais do modelo
    std::vector<std::shared_ptr<MaterialAsset>> materials;  
    // lookup rápido: materialKey -> index
    std::unordered_map<std::string, uint32_t> materialsLUT; //Lookup

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