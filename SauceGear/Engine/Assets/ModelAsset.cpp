#include "ModelAsset.h"
#include "../Serializer/ModelSerializer.h"

void ModelAsset::Reload() {
    LOG_INFO("Reloading model: {}", path); 
    auto fresh = ModelSerializer::Load(path);

    // 1 Atualiza a hierarquia (mantendo ponteiro raiz)
    if (!root)  
        root = fresh->root; 
    else  
        root->CopyFrom(*fresh->root); 

    // 2️ Atualiza meshes UMA A UMA
    for (size_t i = 0; i < meshes.size(); ++i) {
        meshes[i]->ReloadFrom(*fresh->meshes[i]);
    }

    lastWrite = std::filesystem::last_write_time(path);
}
