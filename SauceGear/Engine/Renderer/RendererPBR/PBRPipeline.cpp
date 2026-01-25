#include "PBRPipeline.h" 
#include "../../Graphics/Renderer.h"
#include "../../Graphics/SharedDepthStencil.h"

#include "Passes/GeometryPass.h"
#include "Passes/DeferredLightingPass.h"
#include "Passes/ForwardPass.h"
#include "Passes/SkyboxPass.h"
#include "Passes/TransparentPass.h"
#include "Passes/SSAOPass/SSAOPass.h"

#include "../LightPass/LightPass.h"
#include "../PostProcessPass/PostProcess.h"
#include "../RenderDebugSettings.h"

#include "../Outline/OutlinePass.h"

#include "../../Materials/MaterialLibrary.h"
#include "../Shaders/ShaderLibrary.h"
  
void PBRPipeline::Init() {
    std::cout << "initPipeline" << std::endl; 

    lightPass    = new LightPass(); 
    geometryPass = new GeometryPass(&ShaderLibrary::Get("PBR_GBuffer"));
    lightingPass = new DeferredLightingPass(
        &ShaderLibrary::Get("PBR_IBL"),
        &ShaderLibrary::Get("PBR_Directional"),
        &ShaderLibrary::Get("PBR_Point")
    ); 
    skyboxPass  = new SkyboxPass(&ShaderLibrary::Get("Skybox")); 
    postProcess = new PostProcess();
    outlinePass = new OutlinePass(&ShaderLibrary::Get("Outline"));
    ssaoPass    = new SSAOPass(&ShaderLibrary::Get("SSAO"), &ShaderLibrary::Get("SSAOBlur"));

    // ** FBOs **
    const unsigned int width = GEngine->window->GetWidth();
    const unsigned int height = GEngine->window->GetHeight();  
    sharedDepthStencil = new SharedDepthStencil(width, height);

    gBuffer = new Framebuffer(width, height, {
        {FramebufferTextureType::Position},            // 0
        {FramebufferTextureType::Albedo},  //HDR       // 1 (RGB = baseColor, A = ?)            //Legacy ::Albedo
        {FramebufferTextureType::Normal},              // 2
        {FramebufferTextureType::MetallicRoughnessAO}, // 3 (R = metallic,G = roughness,B = ao, A = ?)
    }, sharedDepthStencil->GetRBO(), true );
    
    lightingBuffer = new Framebuffer(width, height, { {FramebufferTextureType::HDR} }, sharedDepthStencil->GetRBO(), true); //ColorRGB  

    ppBuffer = new Framebuffer(width, height, { {FramebufferTextureType::HDR} }, true);
    
    //SAAO
    ssaoBuffer = new Framebuffer( width/2, height/2, { 
        { FramebufferTextureType::REDFloat }
    });
    ssaoBlurBuffer = new Framebuffer( width/2, height/2, {
        { FramebufferTextureType::REDFloat }
    });


    // ** IBL **
    ibl = IBLManager::EnsureIBL(currentHDR, cacheDir); 


    // *FBO_Biceps*
    GEngine->renderer->frameScreen = lightingBuffer;
}


void PBRPipeline::Render(Scene& scene) {
    auto& engineSettings = GetEngineSettings();
    auto& debug = engineSettings.renderDebug;
    //=============================================TimeLine============================================================
    HandleFBOs();                       //ibl = DayNightSystem::GetSkyboxFront();

    // Render Light and Shadows
    lightPass->Update(debug.Shadow);
    // GBUFFER + STENCIL
    geometryPass->Execute(scene, *gBuffer);
     
    ssaoPass->Execute(*gBuffer, *ssaoBuffer, *ssaoBlurBuffer);

    // Deferred Lighting                    (usa ssao)
    lightingPass->Execute(scene, *lightingBuffer, *gBuffer, ibl);
    // outline usa depth + stencil do geometry
    if (GetEngineSettings().renderDebug.outlineSys) outlinePass->Execute(scene, *lightingBuffer);
    // Forward  
    forwardPass->Execute(scene, *gBuffer, *lightingBuffer);
    // Skybox 
    if (GetEngineSettings().renderDebug.Skybox) skyboxPass->Execute(*GEngine->mainCamera, ibl.envCubemap, *lightingBuffer);  //shaders.skybox


    //========================================BUFFER RENDER============================================================ 

    //----------------------------- New Debug Edit ------------------------------------------------------------
    switch (debug.viewMode) {
    case RenderViewMode::FinalLighting:
        GEngine->renderer->GetTextureRendered = lightingBuffer->GetTexture(0);
        break;

    case RenderViewMode::Position:
        GEngine->renderer->GetTextureRendered = gBuffer->GetTexture(0);
        break;

    case RenderViewMode::Albedo:
        GEngine->renderer->GetTextureRendered = gBuffer->GetTexture(1);
        break;

    case RenderViewMode::Normal:
        GEngine->renderer->GetTextureRendered = gBuffer->GetTexture(2);
        break;

    case RenderViewMode::MRA:
        GEngine->renderer->GetTextureRendered = gBuffer->GetTexture(3);
        break;

    case RenderViewMode::Depth:
        //GEngine->renderer->GetTextureRendered = gBuffer->get;     //GEngine->renderer->GetTextureRendered = gBuffer->GetDepthTexture();
        break;

    case RenderViewMode::AO:
        GEngine->renderer->GetTextureRendered = ssaoBlurBuffer->GetTexture(0);
        break;
    }
    //----------------------------------------------------------------------------------------------------------

    //GEngine->renderer->GetTextureRendered = lightingBuffer->GetTexture(0);  
    //GEngine->renderer->GetTextureRendered = gBuffer->GetTexture(1);

    //  POST PROCESSING
    if (engineSettings.renderDebug.postProcess)  postProcess->Execute(scene, *ppBuffer);
    //Finish, to next fase
    postProcess->Finish(*ppBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void PBRPipeline::HandleFBOs() {
    if (!lightingBuffer || !gBuffer) Init();
    //Resize -> if true, all FBOs will need to be updated
    if (lightingBuffer->GetWidth() != gBuffer->GetWidth() || lightingBuffer->GetHeight() != gBuffer->GetHeight()) {
        // New Size
        unsigned int w = lightingBuffer->GetWidth();        unsigned int h = lightingBuffer->GetHeight();
        
        sharedDepthStencil->Resize(w, h); //always first_ in order

        gBuffer->Resize(w, h);
        ppBuffer->Resize(w, h);
        lightingBuffer->Resize(w, h);
         
        ssaoBuffer->Resize(w/2, h/2);
        ssaoBlurBuffer->Resize(w/2, h/2); 
    }
} 
  
  
void PBRPipeline::Shutdown() {
    if (gBuffer) { delete gBuffer; gBuffer = nullptr; }
    if (lightingBuffer) { delete lightingBuffer; lightingBuffer = nullptr; }
    if (ppBuffer) { delete ppBuffer; ppBuffer = nullptr; }
    if (ssaoBuffer) { delete ssaoBuffer; ssaoBuffer = nullptr; }
    if (ssaoBlurBuffer) { delete ssaoBlurBuffer; ssaoBlurBuffer = nullptr; } 
    IBLManager::Destroy(ibl);
}
 