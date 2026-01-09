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
            { "Albedo",    { ParamDef::Texture, 0 } },
            { "Normal",    { ParamDef::Texture, 1 } },
            { "Metallic",  { ParamDef::Float,   2 } },
            { "Roughness", { ParamDef::Float,   3 } }
        };
    }
};
