#pragma once
#include "../ECS/Reflection/Macros.h"
#include <cstdint>

//Enums refletidos devem viver em .cpp, não em .h.   para evitar
//  múltiplas definições
//  ou símbolos inconsistentes
//  ou erro de linker / ODR
enum class RenderViewMode : int {
    FinalLighting = 0,
    Albedo,
    Normal,
    Position,
    MRA,
    Depth
}; 

// Apenas declaração (não define!)
struct EnumInfo;
extern EnumInfo RenderViewMode_EnumInfo;

struct RenderDebugSettings {
    RenderViewMode viewMode = RenderViewMode::FinalLighting;
    bool IBLAmbient     = true;
    bool SunLight       = true;
    bool PointLights    = true;
    bool Shadow         = true;

    bool Skybox         = true;

    bool postProcess    = false;

    std::string stgTeste;

    REFLECT_CLASS(RenderDebugSettings) {
        REFLECT_ENUM_FIELD(viewMode, RenderViewMode, EditorWidget::EnumCombo)
        REFLECT_HEADER("lights -> IRadiance and Radiance");
        REFLECT_FIELD(IBLAmbient);
        REFLECT_FIELD(SunLight);
        REFLECT_FIELD(PointLights);
        REFLECT_FIELD(Shadow);
        REFLECT_HEADER("Environment");
        REFLECT_FIELD(Skybox)
        REFLECT_HEADER("PostProcess");
        REFLECT_FIELD(postProcess)
        REFLECT_HEADER("Debug");
        REFLECT_FIELD(stgTeste)
    }
};

struct EngineSettings {
    RenderDebugSettings renderDebug;
};

inline EngineSettings& GetEngineSettings() {
    static EngineSettings settings;
    return settings;
}
