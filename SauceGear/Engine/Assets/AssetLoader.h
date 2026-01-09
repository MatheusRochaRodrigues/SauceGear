#pragma once 
#include <memory>
#include <string>

template<typename T>
std::shared_ptr<T> LoadAsset(const std::string& path);


/*
EXAMPLE USE

#include "Assets/AssetLoader.h"

auto mat = LoadAsset<MaterialAsset>(
    "Assets/Materials/wall.material.json"
);

auto shader = LoadAsset<ShaderAsset>(
    "Shaders/pbr.glsl"
);


*/