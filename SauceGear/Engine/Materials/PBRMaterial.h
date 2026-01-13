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
            { "Albedo",    { ParamDef::Texture, 0, /*slider*/ 0.0f, 1.0f, true } },
            { "Normal",    { ParamDef::Texture, 1, /*slider*/ 0.0f, 1.0f, true } },
            { "Metallic",  { ParamDef::Float,   2, /*slider*/ 0.0f, 1.0f, true } },
            { "Roughness", { ParamDef::Float,   3, /*slider*/ 0.0f, 1.0f, true } }
        };
    }
};
