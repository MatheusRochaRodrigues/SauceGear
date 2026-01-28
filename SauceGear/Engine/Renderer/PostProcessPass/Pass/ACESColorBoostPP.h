#pragma once
#include "../PostProcessPass.h"
#include "../../../Graphics/Shader.h"
#include "../../RenderDebugSettings.h"

class ACESColorBoostPP : public PostProcessPass {
public:
    ACESColorBoostPP() {
        shader = new Shader(
            "PostProcess/post.vs",
            "PostProcess/ACESColorBoost.fs"
        );
    }

    void Apply() override {
        auto& engineSettings = GetEngineSettings();
        auto& debug = engineSettings.renderDebug;


        shader->use();/*
        shader->setFloat("uExposure", 1.2f);
        shader->setFloat("uSaturation", 1.25f);
        shader->setFloat("uContrast", 1.1f);*/


        shader->setFloat("uExposure", debug.Exposure);
        shader->setFloat("uSaturation", debug.Saturation);
        shader->setFloat("uContrast", debug.Contrast);
    }
};
