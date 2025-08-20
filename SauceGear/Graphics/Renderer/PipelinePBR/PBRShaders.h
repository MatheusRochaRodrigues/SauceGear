#pragma once
#include "../Graphics/Shader.h"

struct PBRShaders {
    // G-Buffer
    Shader gbufferVSFS = Shader("Shaders/PBR/pbr_gbuffer.vs", "Shaders/PBR/pbr_gbuffer.fs");

    // Lighting (deferred)
    Shader dirLight = Shader("Shaders/PBR/pbr_deferred_dir.vs", "Shaders/PBR/pbr_deferred_dir.fs");     // fullscreen quad
    Shader pointLight = Shader("Shaders/PBR/pbr_deferred_light_volume.vs", "Shaders/PBR/pbr_deferred_point.fs");

    // IBL build
    Shader equirect = Shader("Shaders/PBR/cubemap.vs", "Shaders/PBR/equirectangular_to_cubemap.fs");
    Shader irradiance = Shader("Shaders/PBR/cubemap.vs", "Shaders/PBR/irradiance_convolution.fs");
    Shader prefilter = Shader("Shaders/PBR/cubemap.vs", "Shaders/PBR/prefilter.fs");
    Shader brdf = Shader("Shaders/PBR/brdf.vs", "Shaders/PBR/brdf.fs");

    // Background
    Shader skybox = Shader("Shaders/PBR/background.vs", "Shaders/PBR/background.fs");
};
