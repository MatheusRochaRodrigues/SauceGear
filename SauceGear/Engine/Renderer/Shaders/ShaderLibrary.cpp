#include "ShaderLibrary.h"
#include <stdexcept>

std::unordered_map<std::string, Shader> ShaderLibrary::shaders;

void ShaderLibrary::Init() {

    // === GBUFFER ===
    shaders["PBR_GBuffer"] = Shader(
        "PBR/DeferredShadingPBR/pbr_gbuffer.vs",
        "PBR/DeferredShadingPBR/pbr_gbuffer.fs"
    );
    std::cout << "passou1" << std::endl;

    // === IBL AMBIENT ===
    shaders["PBR_IBL"] = Shader(
        "PBR/DeferredShadingPBR/fullscreen.vs",
        "PBR/DeferredShadingPBR/pbr_ibl.fs"
    );
    std::cout << "passou2" << std::endl;

    // === DIRECTIONAL LIGHT ===
    shaders["PBR_Directional"] = Shader(
        "PBR/DeferredShadingPBR/fullscreen.vs",
        "PBR/DeferredShadingPBR/pbr_deferred_directional.fs",
        {}, true
    );
    std::cout << "passou3" << std::endl;

    // === POINT LIGHT ===
    shaders["PBR_Point"] = Shader(
        "PBR/DeferredShadingPBR/pbr_deferred_light_volume.vs",
        "PBR/DeferredShadingPBR/pbr_deferred_point.fs"
    ); 
    std::cout << "passou4" << std::endl;

    // === SKYBOX ===
    shaders["Skybox"] = Shader(
        "PBR/background.vs",
        "PBR/background.fs"
    );
    std::cout << "passou5" << std::endl;

    // === IBL PRECOMPUTE ===
    shaders["HDR_ToCubemap"] = Shader(
        "PBR/cubemap.vs",
        "PBR/IBL/equirectangular_to_cubemap.fs"
    );
    std::cout << "passou6" << std::endl;

    shaders["IBL_Irradiance"] = Shader(
        "PBR/cubemap.vs",
        "PBR/IBL/irradiance_convolution.fs"
    );
    std::cout << "passou7" << std::endl;

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
