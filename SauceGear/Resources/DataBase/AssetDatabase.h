#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <typeindex>
#include <stdexcept>
#include <iostream>

class AssetDatabase {
public:
    // Retorna uma referência a um placeholder (shared_ptr vazio) para você preencher depois
    template <typename T>
    static std::shared_ptr<T>& RegisterPlaceholder(const std::string& key) {
        auto it = assets.find(key);
        if (it != assets.end()) {
            if (it->second.type != std::type_index(typeid(T))) {
                throw std::runtime_error("Erro: Asset existente tem tipo diferente do solicitado para " + key);
            }
            auto existing = it->second.ptr.lock();
            if (existing) {
                return *reinterpret_cast<std::shared_ptr<T>*>(&existing);
            }
        }

        // Cria um shared_ptr vazio e registra
        auto placeholder = std::make_shared<T>(nullptr);
        assets[key] = { placeholder, std::type_index(typeid(T)) };
        return *reinterpret_cast<std::shared_ptr<T>*>(&assets[key].ptr);
    }

    // Load clássico: retorna asset já criado ou cria com um construtor padrão T(path)
    template <typename T>
    static std::shared_ptr<T> Load(const std::string& key) {
        auto it = assets.find(key);
        if (it != assets.end()) {
            auto existing = it->second.ptr.lock();
            if (existing) {
                if (it->second.type != std::type_index(typeid(T))) {
                    throw std::runtime_error("Erro: Tipo diferente do solicitado para " + key);
                }
                return std::static_pointer_cast<T>(existing);
            }
        }

        // Cria um novo asset e registra
        auto newAsset = std::make_shared<T>(key);
        assets[key] = { newAsset, std::type_index(typeid(T)) };
        return newAsset;
    }

    // Load com função de criação customizada (pode passar flags, diretórios etc)
    template <typename T, typename Factory>
    static std::shared_ptr<T> Load(const std::string& key, Factory createFunc) {
        auto it = assets.find(key);
        if (it != assets.end()) {
            auto existing = it->second.ptr.lock();
            if (existing) {
                if (it->second.type != std::type_index(typeid(T))) {
                    throw std::runtime_error("Erro: Tipo diferente do solicitado para " + key);
                }
                return std::static_pointer_cast<T>(existing);
            }
        }

        auto newAsset = createFunc(); // cria asset fora
        assets[key] = { newAsset, std::type_index(typeid(T)) };
        return newAsset;
    }

    // Unload remove do database; se ninguém mais referenciar, objeto será destruído
    template <typename T>
    static void Unload(const std::string& key) {
        auto it = assets.find(key);
        if (it != assets.end() && it->second.type == std::type_index(typeid(T))) {
            assets.erase(it);
        }
    }

private:
    struct AssetEntry {
        std::weak_ptr<void> ptr;      // Não impede destruição
        std::type_index type;
    };

    static std::unordered_map<std::string, AssetEntry> assets;
};

// Definição do static
std::unordered_map<std::string, AssetDatabase::AssetEntry> AssetDatabase::assets;


namespace AssetDatabaseUtils {
    inline std::string MakeMaterialKey(const std::string& modelName, const std::string& materialName) {
        return modelName + "::" + materialName; // separador :: para evitar colisões
    }
}