#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <typeindex>
#include <stdexcept>
#include <iostream>
 

class AssetDatabase {
public:
    // Load clássico: retorna asset já criado ou cria com T(path)
    template <typename T>
    static std::shared_ptr<T> Load(const std::string& key) {
        auto it = assets.find(key);
        if (it != assets.end()) { 
            if (auto existing = it->second.ptr.lock()) {
                if (it->second.type != std::type_index(typeid(T))) {
                    throw std::runtime_error("Erro: Tipo diferente do solicitado para " + key);
                }
                return std::static_pointer_cast<T>(existing);
            }
        }

        // Cria e registra
        auto newAsset = std::make_shared<T>(key);
        assets[key] = { newAsset, std::type_index(typeid(T)) };
        return newAsset;
    }

    // Load com função de criação customizada (quando precisa de flags, configs, etc.)
    template <typename T, typename Factory>
    static std::shared_ptr<T> Load(const std::string& key, Factory createFunc) {
        auto it = assets.find(key);
        if (it != assets.end()) { 
            if (auto existing = it->second.ptr.lock()) {
                if (it->second.type != std::type_index(typeid(T))) {
                    throw std::runtime_error("Erro: Tipo diferente do solicitado para " + key);
                }
                return std::static_pointer_cast<T>(existing);
            }
        }

        auto newAsset = createFunc();
        assets[key] = { newAsset, std::type_index(typeid(T)) };
        return newAsset;
    }

    // Remove do database; se ninguém mais tiver referência, o asset morre
    template <typename T>
    static void Unload(const std::string& key) {
        auto it = assets.find(key);
        if (it != assets.end() && it->second.type == std::type_index(typeid(T))) {
            assets.erase(it);
        }
    }

private:
    struct AssetEntry {
        std::weak_ptr<void> ptr;   // não segura vida do objeto
        std::type_index type;

        AssetEntry() : ptr(), type(typeid(void)) {} // default (necessário para unordered_map)
        AssetEntry(std::weak_ptr<void> p, std::type_index t) : ptr(std::move(p)), type(t) { }
    };

    inline static std::unordered_map<std::string, AssetEntry> assets{};
};

 

namespace AssetDatabaseUtils {
    inline std::string MakeMaterialKey(const std::string& modelName, const std::string& materialName) {
        return modelName + "::" + materialName; // separador :: para evitar colisões
    }
}
