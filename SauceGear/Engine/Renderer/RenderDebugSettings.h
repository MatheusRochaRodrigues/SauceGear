#pragma once
#include "../ECS/Reflection/Macros.h"
#include <cstdint>
#include <glm/glm.hpp>

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
    Depth,
    AO,
    Fog
}; 

// Apenas declaração (não define!)
struct EnumInfo;
extern EnumInfo RenderViewMode_EnumInfo;


enum class SkyboxMode : int {
    Skybox = 0,
    IBLEnvirolnment, 
};
extern EnumInfo SkyboxMode_EnumInfo;



enum class HDRMode : int {
    ACESFilm = 0,
    HDR,
};
extern EnumInfo HDRMode_EnumInfo;





struct RenderDebugSettings {
    RenderViewMode viewMode = RenderViewMode::FinalLighting;
    bool IBLAmbient     = true;
    bool SunLight       = true;
    bool PointLights    = true;
    bool Shadow         = true;
    
    //SSAO_Params
    bool  SSAO          = true;
    int   sKernelSize   = 32;
    float sRadius       = 0.5f;    
    float sBias         = 0.025;

    //Fog
    bool FogEnabled     = true;
    float FogDensity    = 1.0f;
    float FogNear       = 25.0f;
    float FogFar        = 110.0f;
    glm::vec3 FogColorNear = glm::vec3(0.096f, 0.351f, 0.475f);
    glm::vec3 FogColorFar  = glm::vec3(0.318f, 0.617f, 0.596f);


    //HDR
    HDRMode hdrMode = HDRMode::ACESFilm;
    bool GammaHDR_correct = true;
    float Exposure = 1.0f;
    float Saturation = 1.0f;
    float Contrast = 1.1f;
    float Power = 1.0f;

    //----------------------------------------- 
    SkyboxMode skyMode = SkyboxMode::IBLEnvirolnment;
    bool Skybox         = true;

    bool postProcess    = false;

    bool outlineSys = false;                                        //bool OutlDepth  = true;

    std::string stgTeste;

    REFLECT_CLASS(RenderDebugSettings) {
        REFLECT_ENUM_FIELD(viewMode, RenderViewMode, EditorWidget::EnumCombo)
        REFLECT_SPACE();

        REFLECT_HEADER("lights -> IRadiance and Radiance");
        REFLECT_FIELD(IBLAmbient);
        REFLECT_FIELD(SunLight);
        REFLECT_FIELD(PointLights);
        REFLECT_FIELD(Shadow);

        REFLECT_SPACE();
        REFLECT_HEADER("SSAO Config");
        REFLECT_FIELD(SSAO);
        REFLECT_INT_SLIDER(sKernelSize, 1, 64);
        REFLECT_FLOAT_SLIDER(sRadius, 0.0f, 2.0f);  
        REFLECT_FLOAT_SLIDER(sBias, 0.001f, 0.04f);
        REFLECT_FIELD(Power);
        REFLECT_SPACE();

         
        REFLECT_HEADER("FOG Config");
        REFLECT_FIELD(FogEnabled);
        REFLECT_FIELD(FogDensity);
        REFLECT_FLOAT_SLIDER(FogNear, 0.0f, 200.0f);
        REFLECT_FLOAT_SLIDER(FogFar, 0.0f, 500.0f); 
        REFLECT_FIELD_COLOR(FogColorNear);
        REFLECT_FIELD_COLOR(FogColorFar);
        REFLECT_SPACE();



        REFLECT_HEADER("Environment");
        REFLECT_ENUM_FIELD(skyMode, SkyboxMode, EditorWidget::EnumCombo)
        REFLECT_FIELD(Skybox)
        REFLECT_SPACE();

        REFLECT_HEADER("PostProcess"); 
        REFLECT_FIELD(postProcess)

        REFLECT_HEADER("HDR / Tonemapping");
        REFLECT_FIELD(GammaHDR_correct);
        REFLECT_ENUM_FIELD(hdrMode, HDRMode, EditorWidget::EnumCombo)
        REFLECT_FLOAT_SLIDER(Exposure, 0.0f, 5.0f);   // energia
        REFLECT_FLOAT_SLIDER(Saturation, 0.0f, 2.0f);   // cromaticidade
        REFLECT_FLOAT_SLIDER(Contrast, 0.5f, 2.0f);   // punch

        REFLECT_SPACE();

        REFLECT_HEADER("Outline System");
        REFLECT_FIELD(outlineSys) 
        REFLECT_SPACE();

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
