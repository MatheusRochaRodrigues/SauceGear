#pragma once
#include <memory>
#include <string>

class MaterialAsset;

class MaterialSerializer {
public:
    static std::shared_ptr<MaterialAsset> Load(const std::string& path);
    static void Save(const MaterialAsset& mat, const std::string& path);
};


/*  exemple use

auto material = AssetDatabase::Load<MaterialAsset>(
    "Assets/Materials/wall.material.json",
    [&] {
        return MaterialSerializer::Load("Assets/Materials/wall.material.json");
    }
); 

*/  

/*

📄 Exemplo real (metal_red.gmat)
{
  "base": "PBR_Default",
  "defaults": {
    "Albedo": [1.0, 0.0, 0.0],
    "Metallic": 1.0,
    "Roughness": 0.2
  }
}


*/