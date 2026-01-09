#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
#include <typeindex>
#include <stdexcept>
#include <iostream>
#include "../Core/Log.h"
#include "../Assets/IAsset.h"

class AssetDatabase {
public:
    template<typename T> 
    static std::shared_ptr<T> Load( const std::string& path, std::function<std::shared_ptr<T>()> loader ) {
        auto& map = GetMap();
        auto it = map.find(path);
        if (it != map.end())
            return std::static_pointer_cast<T>(it->second);

        auto asset = loader();
        asset->path = path;
        asset->lastWrite = std::filesystem::last_write_time(path);
        map[path] = asset;

        return asset;
    }

    static void Update() {
        auto& map = GetMap();
        for (auto& [path, asset] : map) {
            auto t = std::filesystem::last_write_time(path);
            if (t != asset->lastWrite) {
                asset->Reload();
                LOG_INFO("Asset hot-reloaded: {}", path);
            }
        }
    }

private:
    static std::unordered_map<std::string, std::shared_ptr<IAsset>>& GetMap() {
        static std::unordered_map<std::string, std::shared_ptr<IAsset>> map;
        return map;
    }
};




/*

auto mat = AssetDatabase::Load<MaterialAsset>(
    "Materials/wall.material.json",
    [&] { return MaterialSerializer::Load("Materials/wall.material.json"); }
);

auto shader = AssetDatabase::Load<ShaderAsset>(
    "Shaders/pbr.glsl",
    [&] { return ShaderSerializer::Load("Shaders/pbr.glsl"); }
);


*/