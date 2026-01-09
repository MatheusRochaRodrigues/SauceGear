#pragma once 
#include <unordered_map>
#include "../Instancing/MaterialInstance.h"
#include "../Materials/TextureCache.h" 
#include "../Materials/MaterialBase.h" 
#include <iostream>
#include <type_traits>
#include <variant>
 
#define LOG_WARN(fmt, ...) \
    std::cout << "[WARN] " << fmt << std::endl

class MaterialBinder {
public:
    static std::shared_ptr<Texture> Resolve( const std::string& name, const MaterialBase::ParamDef& def, const MaterialInstance* inst ) {
        if (inst) {
            auto it = inst->overrides.find(name);
            if (it != inst->overrides.end()) {
                if (auto t = std::get_if<std::shared_ptr<Texture>>(&it->second.data))
                    return *t;
                if (auto c = std::get_if<glm::vec4>(&it->second.data))
                    return TextureCache::Get().GetSolidColor(*c);
            }
        }
        return TextureCache::Get().GetSolidColor({ 1,1,1,1 });
    }
     
    static void Bind(MaterialInstance& inst, MaterialAsset& asset, MaterialBase& base) {
        for (auto& [name, def] : base.layout) {

            const MaterialInstance::Value* value = nullptr;

            if (inst.overrides.find(name) != inst.overrides.end())
                value = &inst.overrides[name];
            else if (asset.defaults.find(name) != asset.defaults.end())
                value = &asset.defaults[name];

            if (!value)
                continue;

            bool applied = false;

            std::visit([&](auto&& value) {
                using T = std::decay_t<decltype(value)>;

                if constexpr (std::is_same_v<T, float>) {
                    if (def.type == MaterialBase::ParamDef::Float) {
                        base.shader->setFloat(name, value);
                        applied = true;
                    }
                }
                else if constexpr (std::is_same_v<T, glm::vec3>) {
                    if (def.type == MaterialBase::ParamDef::Vec3) {
                        base.shader->setVec3(name, value);
                        applied = true;
                    }
                }
                else if constexpr (std::is_same_v<T, glm::vec4>) {
                    if (def.type == MaterialBase::ParamDef::Vec4) {
                        base.shader->setVec4(name, value);
                        applied = true;
                    }
                }
                else if constexpr (std::is_same_v<T, std::shared_ptr<Texture>>) {
                    if (def.type == MaterialBase::ParamDef::Texture) {
                        base.shader->setTexture2D(name, value->ID, def.unit);
                        applied = true;
                    }
                }
                }, value->data);

            if (!applied) {
                LOG_WARN("Material param '{}' type mismatch", name);
            }
        }
    }




private:
    inline const char* ToString(MaterialBase::ParamDef::Type t) {
        switch (t) {
        case MaterialBase::ParamDef::Float:   return "Float";
        case MaterialBase::ParamDef::Vec3:    return "Vec3";
        case MaterialBase::ParamDef::Vec4:    return "Vec4";
        case MaterialBase::ParamDef::Texture: return "Texture";
        default:                              return "Unknown";
        }
    }

    static const char* VariantTypeToString(
        const std::variant<
        std::monostate,
        float,
        glm::vec3,
        glm::vec4,
        std::shared_ptr<Texture>
        >& v
    ) {
        return std::visit([](auto&& value) -> const char* {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, std::monostate>)                return "None";
            else if constexpr (std::is_same_v<T, float>)                    return "Float";
            else if constexpr (std::is_same_v<T, glm::vec3>)                return "Vec3";
            else if constexpr (std::is_same_v<T, glm::vec4>)                return "Vec4";
            else if constexpr (std::is_same_v<T, std::shared_ptr<Texture>>) return "Texture";
            else                                                            return "Unknown";
            }, v);
    }


};










 

 