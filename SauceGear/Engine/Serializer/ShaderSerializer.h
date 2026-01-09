//#pragma once
//#include "AssetLoader.h"
//#include "AssetDatabase.h"
//
//#include "MaterialAsset.h"
//#include "../Serializer/MaterialSerializer.h"
//
//#include "ShaderAsset.h"
//
//// 🔹 Material (JSON)
//template<>
//std::shared_ptr<MaterialAsset>
//LoadAsset<MaterialAsset>(const std::string& path) {
//    return AssetDatabase::Load<MaterialAsset>(
//        path,
//        [&] { return MaterialSerializer::Load(path); }
//    );
//}
//
//// 🔹 Shader (direto do arquivo)
//template<>
//std::shared_ptr<ShaderAsset>
//LoadAsset<ShaderAsset>(const std::string& path) {
//    return AssetDatabase::Load<ShaderAsset>(
//        path,
//        [&] { return ShaderAsset::Load(path); }
//    );
//}


/*

auto shader = LoadAsset<ShaderAsset>("Shaders/pbr.glsl");
auto material = LoadAsset<MaterialAsset>("Materials/wall.material.json");


*/