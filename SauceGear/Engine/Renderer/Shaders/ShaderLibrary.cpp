#include "ShaderLibrary.h"
#include <stdexcept>

std::unordered_map<std::string, Shader> ShaderLibrary::shaders;

void ShaderLibrary::Init() { 
    //================================================================ Shadow ================================================================  
    // === SHADOW CASCADE SUN LIGHT ===
    shaders["ShadowCascadeSun"] = Shader(
        "Shadows/ShadowMapCasc.vs", 
        "Shadows/ShadowMapCasc.gs", 
        "Shadows/ShadowMapCasc.fs"
    );

    // === SHADOW DIRECTIONAL LIGHT ===
    shaders["ShadowDirectional"] = Shader(
        "Shadows/ShadowMapD.vs", 
        "Shadows/ShadowMapD.fs"
    );

    // === SHADOW POINT LIGHT ===
    shaders["ShadowPoint"] = Shader(
        "Shadows/ShadowMapP.vs", 
        "Shadows/ShadowMapP.gs", 
        "Shadows/ShadowMapP.fs"
    );


    //============================================================= Renderer PBR =============================================================
    // === GBUFFER ===
    shaders["PBR_GBuffer"] = Shader(
        "PBR/DeferredShadingPBR/pbr_gbuffer.vs",
        "PBR/DeferredShadingPBR/pbr_gbuffer.fs"
    ); 

    // === IBL AMBIENT ===
    shaders["PBR_IBL"] = Shader(
        "PBR/DeferredShadingPBR/fullscreen.vs",
        "PBR/DeferredShadingPBR/pbr_ibl.fs"
    ); 

    // === DIRECTIONAL LIGHT ===
    shaders["PBR_Directional"] = Shader(
        "PBR/DeferredShadingPBR/fullscreen.vs",
        "PBR/DeferredShadingPBR/pbr_deferred_directional.fs",
        {}, true
    ); 

    // === POINT LIGHT ===
    shaders["PBR_Point"] = Shader(
        "PBR/DeferredShadingPBR/pbr_deferred_light_volume.vs",
        "PBR/DeferredShadingPBR/pbr_deferred_point.fs"
    );  

    // === SKYBOX ===
    shaders["Skybox"] = Shader(
        "PBR/background.vs",
        "PBR/background.fs"
    ); 

    // === IBL PRECOMPUTE ===
    shaders["HDR_ToCubemap"] = Shader(
        "PBR/cubemap.vs",
        "PBR/IBL/equirectangular_to_cubemap.fs"
    ); 

    shaders["IBL_Irradiance"] = Shader(
        "PBR/cubemap.vs",
        "PBR/IBL/irradiance_convolution.fs"
    ); 

    shaders["IBL_Prefilter"] = Shader(
        "PBR/cubemap.vs",
        "PBR/IBL/prefilter.fs"
    );

    shaders["IBL_BRDF"] = Shader(
        "PBR/IBL/brdf.vs",
        "PBR/IBL/brdf.fs"
    );
}

Shader& ShaderLibrary::Get(const std::string& name) {
    auto it = shaders.find(name);
    if (it == shaders.end())
        throw std::runtime_error("ShaderLibrary::Get -> Shader not found: " + name);
    return it->second;
}

bool ShaderLibrary::Exists(const std::string& name) {
    return shaders.find(name) != shaders.end();
}







//
/*                              DeferredShading Pure/ Classic withouth PBR

    //GBuffer
    GEngine->renderer->GetGBufferShader = new Shader("DeferredShading/gBuffer.vs", "DeferredShading/gBuffer.fs");
    //Lighting pos GBuffer
    GEngine->renderer->GetLightingShader = new Shader("DeferredShading/DeferredShading.vs", "DeferredShading/DeferredShading.fs");
    GEngine->renderer->GetSunLightingShader = new Shader("DeferredShading/DeferredShadingSun.vs", "DeferredShading/DeferredShadingSun.fs");

*/