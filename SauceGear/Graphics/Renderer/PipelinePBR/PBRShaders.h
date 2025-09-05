#pragma once
#include "../Graphics/Shader.h"

struct PBRShaders {
    // G-Buffer
    Shader gbuffer = Shader(
        "PBR/DeferredShadingPBR/pbr_gbuffer.vs", 
        "PBR/DeferredShadingPBR/pbr_gbuffer.fs"
    );

    //  iblAmbientShader G-Buffer
    Shader iblAmbientShader = Shader(
        "PBR/DeferredShadingPBR/fullscreen.vs", 
        "PBR/DeferredShadingPBR/pbr_ibl.fs"
    );

    // Lighting (deferred) 
    Shader dirLight = Shader(
        "PBR/DeferredShadingPBR/fullscreen.vs",
        "PBR/DeferredShadingPBR/pbr_deferred_directional.fs",
        {}, true
    );     // fullscreen quad

    Shader pointLight = Shader(
        "PBR/DeferredShadingPBR/pbr_deferred_light_volume.vs", 
        "PBR/DeferredShadingPBR/pbr_deferred_point.fs"); 

    // Background 
    Shader skybox = Shader("PBR/background.vs", "PBR/background.fs");
    /*Shader skybox = Shader( 
        "Shaders/PBR/DeferredShadingPBR/skybox.vs", 
        "Shaders/PBR/DeferredShadingPBR/skybox.fs"
    ); */   

    // IBL build
    Shader hdrToCube = Shader(
        "PBR/cubemap.vs", 
        "PBR/IBL/equirectangular_to_cubemap.fs"
    );

    Shader irradiance = Shader(
        "PBR/cubemap.vs", 
        "PBR/IBL/irradiance_convolution.fs"
    );

    Shader prefilter = Shader(
        "PBR/cubemap.vs", 
        "PBR/IBL/prefilter.fs"
    );

    Shader brdf =   Shader(
        "PBR/IBL/brdf.vs", 
        "PBR/IBL/brdf.fs"
    );


};


//O problema é que valores muito baixos de roughness(ex: 0.0 ~0.01) geram highlights extremamente concentrados, quase pontuais.
//Se você tem iluminação baseada em múltiplas amostras(IBL, importance sampling, etc.), isso pode causar fireflies(pontos brancos aleatórios e intensos) porque:
//
//A BRDF especular fica muito estreita - precisa de muitas amostras para convergir.
//
//Se você não amostrar suficiente - valores muito altos escapam - fireflies.
//ISSO EVITA FIRE FLIES

//              SOLUCAO
//// valor vindo de textura ou uniforme
//float roughness = texture(roughnessMap, TexCoords).r;
//
//// Clamping para evitar fireflies
//roughness = max(roughness, 0.04);



// Shaders
//iblAmbientShader = new Shader("fullscreen.vs", "pbr_ibl.fs");   // passo 1
//pbrPointShader = new Shader("pbr_point.vs", "pbr_point.fs");  // passo 2 (instanced sphere)
//skyboxShader = new Shader("skybox.vs", "skybox.fs");

//// IBL (carregar/gerar + cache)
//equirectangularToCubemapShader = Shader("cubemap.vs", "equirectangular_to_cubemap.fs");
//irradianceShader = Shader("cubemap.vs", "irradiance_convolution.fs");
//prefilterShader = Shader("cubemap.vs", "prefilter.fs");
//brdfShader = Shader("brdf.vs", "brdf.fs");