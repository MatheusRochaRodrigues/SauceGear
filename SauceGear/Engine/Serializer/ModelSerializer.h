#pragma once
#include "../Assets/ModelAsset.h"  

class ModelSerializer {
public:
    static std::shared_ptr<ModelAsset> Load(const std::string& path);
};




/*
Example arc

{
  "mesh": "Models/house.fbx",
  "materials": {
    "Wall": "Materials/wall.material.json",
    "Roof": "Materials/roof.material.json"
  }
}

*/