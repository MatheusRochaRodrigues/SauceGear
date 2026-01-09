#include "MaterialSerializer.h"

#include <nlohmann/json.hpp>
#include <fstream>

#include "../Assets/MaterialAsset.h"
#include "../Materials/MaterialLibrary.h"
#include "../Materials/MaterialBase.h"
#include "../Materials/TextureCache.h"
#include "../Assets/AssetDatabase.h"

using json = nlohmann::json;

std::shared_ptr<MaterialAsset> MaterialSerializer::Load(const std::string& path) {
    json j;
    std::ifstream file(path);
    file >> j;

    auto asset = std::make_shared<MaterialAsset>();
    asset->path = path;
    asset->lastWrite = std::filesystem::last_write_time(path);

    // Base material
    std::string baseName = j["base"];
    asset->base = MaterialLibrary::Get(baseName);


    // Defaults
    for (auto& [key, val] : j["values"].items()) {
        MaterialInstance::Value v;

        if (val.is_number_float() || val.is_number_integer()) {
            v.data = val.get<float>();
        }
        else if (val.is_array() && val.size() == 3) {
            v.data = glm::vec3(val[0], val[1], val[2]);
        }
        else if (val.is_array() && val.size() == 4) {
            v.data = glm::vec4(val[0], val[1], val[2], val[3]);
        }
        else if (val.is_string()) {
            v.data = TextureCache::Get().Load(val.get<std::string>());
        }

        asset->defaults[key] = v;
    }

    return asset;
}

void MaterialSerializer::Save(const MaterialAsset& mat, const std::string& path) {
    json j;
    j["base"] = mat.name;

    for (auto& [k, v] : mat.defaults) {
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, float>)
                j["values"][k] = arg;
            else if constexpr (std::is_same_v<T, glm::vec3>)
                j["values"][k] = { arg.x, arg.y, arg.z };
            else if constexpr (std::is_same_v<T, glm::vec4>)
                j["values"][k] = { arg.x, arg.y, arg.z, arg.w };
            }, v.data);
    }

    std::ofstream(path) << j.dump(4);
}
