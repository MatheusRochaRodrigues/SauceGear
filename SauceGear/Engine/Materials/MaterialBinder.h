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
    static void Resolve(MaterialValue& v)
    {
        // Já é textura → ok
        if (std::holds_alternative<std::shared_ptr<Texture>>(v))
            return;

        // Color → solid texture
        if (auto c = std::get_if<Color>(&v)) {
            //v = TextureCache::Get().GetSolidColor( glm::vec4(c->r, c->g, c->b, c->a) );
            v = TextureCache::Get().GetSolidColor(c->value);
            return;
        }

        // float → grayscale solid texture
        if (auto f = std::get_if<float>(&v)) {
            v = TextureCache::Get().GetSolidColor(
                glm::vec4(*f, *f, *f, 1.0f)
            );
            return;
        }

        // vec4 → solid texture
        if (auto c = std::get_if<glm::vec4>(&v)) {
            v = TextureCache::Get().GetSolidColor(*c);
            return;
        }

        // fallback defensivo
        v = TextureCache::Get().GetSolidColor({ 1,1,1,1 });
    } 


     
    static void Bind(MaterialInstance& inst, MaterialAsset& asset, MaterialBase& base) { 
#ifdef DGB
        std::cout << std::endl << "mat dgb " << asset.name << std::endl;
#endif

        for (auto& [name, def] : base.layout) {

            MaterialInstance::Value* value = nullptr;

            if (inst.overrides.find(name) != inst.overrides.end())
                value = &inst.overrides[name];
            else if (asset.defaults.find(name) != asset.defaults.end())
                value = &asset.defaults[name];

            if (!value)
                continue;
             
#ifdef DGB
            std::cout << "1 " << name                   << std::endl;
            std::cout << "2 " << def.type               << std::endl;
            std::cout << "3 " << value->data.index()    << std::endl;
#endif 

            if (def.type == MaterialBase::ParamDef::Type::Texture) Resolve(value->data);

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
                    if (def.type == MaterialBase::ParamDef::Type::Texture) {
                        base.shader->setTexture2D(name, value->ID, def.unit);
                        applied = true;
                    }
                }
            }, value->data);


            if (!applied) {
                std::cout << name << " + " << std::endl;
                LOG_WARN("Material param '{}' type mismatch", name);
            }

#ifdef DGB
            std::cout << "----------------------------------------------------" << std::endl;
#endif

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
        Color,
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
            else if constexpr (std::is_same_v<T, Color>)                    return "Color";
            else if constexpr (std::is_same_v<T, glm::vec3>)                return "Vec3";
            else if constexpr (std::is_same_v<T, glm::vec4>)                return "Vec4";
            else if constexpr (std::is_same_v<T, std::shared_ptr<Texture>>) return "Texture";
            else                                                            return "Unknown";
            }, v);
    }


};










 

 