#pragma once
#include <unordered_map>
#include <string>
#include <variant>
#include <memory>
#include <glm/glm.hpp> 

class Texture;
class MaterialAsset;

using MaterialValue = std::variant<
    std::monostate,
    float,
    glm::vec3,
    glm::vec4,
    std::shared_ptr<Texture>
>; 

class MaterialInstance {
public:
    struct Value {
        MaterialValue data;
    };

    std::string path;       //debug

    std::shared_ptr<MaterialAsset> asset;
    std::unordered_map<std::string, Value> overrides;  //values

    bool dirty = false;

    explicit MaterialInstance(const std::shared_ptr<MaterialAsset>& b);

};


/*


    std::unordered_map<std::string, Value> overrides;

    void SetTexture(const std::string& name, std::shared_ptr<Texture> tex) {
        overrides[name].data = tex;
    }

    void SetColor(const std::string& name, glm::vec4 color) {
        overrides[name].data = color;
    }

*/