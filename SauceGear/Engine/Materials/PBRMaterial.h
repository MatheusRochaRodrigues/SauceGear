#pragma once
#include "MaterialBase.h"
#include "../Renderer/Shaders/ShaderLibrary.h"

class PBRMaterial : public MaterialBase {
public:
    PBRMaterial() {
        domain = MaterialDomain::Deferred;
        shader = &ShaderLibrary::Get("PBR_GBuffer");
        ASSERT(shader && "PBR_GBuffer shader not loaded");
         
        layout = {
            { "Albedo",    {ParamDef::Texture, ParamDef::UIType::Color,  1 } },
            { "Normal",    {ParamDef::Texture, ParamDef::UIType::NONE,   2 } },
            { "Metallic",  {ParamDef::Texture, ParamDef::UIType::Slider, 3, /*slider*/ 0.0f, 1.0f } },
            { "Roughness", {ParamDef::Texture, ParamDef::UIType::Slider, 4, /*slider*/ 0.0f, 1.0f } }
        }; 

    }
};
