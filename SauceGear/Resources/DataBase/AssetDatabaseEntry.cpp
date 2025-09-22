/*
#include "AssetDatabase.h"
std::unordered_map<std::string, std::weak_ptr<void>> AssetDatabase::assets; 

template <typename T>
std::shared_ptr<T> AssetDatabase::Load(const std::string& path) {
    // Verifica se já existe no cache
    auto it = assets.find(path);
    if (it != assets.end()) {
        if (auto existing = it->second.lock()) {
            return std::static_pointer_cast<T>(existing);
        }
    }

    // Se não existe, cria e guarda
    auto newAsset = std::make_shared<T>(path); // Ex: Texture(path)
    assets[path] = newAsset;
    return newAsset;
}

template <typename T>
void AssetDatabase::Unload(const std::string& path) {
    assets.erase(path);
}


/*
template <typename T>
std::shared_ptr<T> AssetDatabase::Load(const std::string& path) {
    // Verifica se já existe no cache
    auto it = assets.find(path);
    if (it != assets.end()) {
        if (auto existing = it->second.lock()) {
            auto typed = std::dynamic_pointer_cast<T>(existing);
            if (!typed) {
                throw std::runtime_error("Asset existente não é do tipo esperado: " + path);
            }
            return typed;
        }
    }

    // Se não existe, cria e guarda
    auto newAsset = std::make_shared<T>(path);
    assets[path] = newAsset;
    return newAsset;
}
*/
