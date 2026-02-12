#include "PBRPipeline.h" 
#include "../../Graphics/Renderer.h"
#include "../../Graphics/SharedDepthStencil.h"
#include "../RenderDebugSettings.h" 
#include "../../Materials/MaterialLibrary.h"
#include "../Shaders/ShaderLibrary.h"

#include "Passes/GeometryPass.h"
#include "Passes/DeferredLightingPass.h"
#include "Passes/ForwardPass.h"
#include "Passes/SkyboxPass.h"
#include "Passes/TransparentPass.h"
#include "Passes/SSAOPass/SSAOPass.h"
#include "Passes/FogPass.h" 
#include "../LightPass/LightPass.h"
#include "../PostProcessPass/PostProcess.h"
#include "../Outline/OutlinePass.h"

#include "../../Core/Profiler/ProfilerMacros.h"
  
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

    postBufferA = new Framebuffer(width, height, { {FramebufferTextureType::HDR} }, true);
    postBufferB = new Framebuffer(width, height, { {FramebufferTextureType::HDR} }, true);
    
    //SAAO
    ssaoBuffer = new Framebuffer( width/2, height/2, { 
        { FramebufferTextureType::REDFloat }
    });
    ssaoBlurBuffer = new Framebuffer( width/2, height/2, {
        { FramebufferTextureType::REDFloat }
    });

    fogPass = new FogPass(&ShaderLibrary::Get("Fog"));

    // ** IBL **
    ibl = IBLManager::EnsureIBL(currentHDR, cacheDir); 


    // *FBO_Biceps*
    GEngine->renderer->frameScreen = lightingBuffer;
    GEngine->renderer->width = width;
    GEngine->renderer->height = height;
}


void PBRPipeline::Render(Scene& scene) {  
    //=============================================TimeLine============================================================
    HandleFBOs();                       //ibl = DayNightSystem::GetSkyboxFront();
     
    {   // Render Light and Shadows
        PROFILE_CPU("LightPass");   
        PROFILE_GPU("LightPass");

        lightPass->Update(GetEngineSettings().renderDebug.Shadow);
    } 
    
    {   // GBUFFER + STENCIL 
        PROFILE_CPU("GeometryPass");
        PROFILE_GPU("GeometryPass");

        gBuffer->Bind();
        geometryPass->Execute(scene);
        gBuffer->Unbind();
    }

    // SSAO - proprio gerenciamento de FBO Interno
    if (GetEngineSettings().renderDebug.SSAO) {
        PROFILE_CPU("SSAOPass");
        PROFILE_GPU("SSAOPass");
        ssaoPass->Execute(*gBuffer, *ssaoBuffer, *ssaoBlurBuffer);
    }

    {   // Deferred Lighting  (usa ssao)
        PROFILE_CPU("DeferredLightingPass");
        PROFILE_GPU("DeferredLightingPass");
        lightingBuffer->Bind();
        lightingPass->Execute(
            scene, *gBuffer, ibl,
            GetEngineSettings().renderDebug.SSAO,
            ssaoBlurBuffer->GetTexture(0)
        );
    } 

    // outline usa depth + stencil do geometry 
    if (GetEngineSettings().renderDebug.outlineSys) {
        PROFILE_CPU("OutlinePass");
        PROFILE_GPU("OutlinePass");
        outlinePass->Execute(scene); 
    }
     
    // Forward  
    //forwardPass->Execute(scene, *gBuffer, *lightingBuffer); 

    // Skybox  
    if (GetEngineSettings().renderDebug.Skybox) {
        PROFILE_CPU("SkyboxPass");
        PROFILE_GPU("SkyboxPass");
        skyboxPass->Execute(
            *GEngine->mainCamera,
            (GetEngineSettings().renderDebug.skyMode == SkyboxMode::Skybox ? 0 : ibl.envCubemap)
        );
    }

    // === FOG PASS (AQUI) ===
    if (GetEngineSettings().renderDebug.FogEnabled) {
        PROFILE_CPU("FogPass");
        PROFILE_GPU("FogPass");
        postBufferA->Bind();
        fogPass->Execute(lightingBuffer, gBuffer, GEngine->mainCamera->Position);
    } 

    // SUN
    skyboxPass->RenderDebugSun();
      

    //========================================BUFFER RENDER============================================================ 
    
    //----------------------------- New Debug Edit ------------------------------------------------------------
    switch (GetEngineSettings().renderDebug.viewMode) {

    case RenderViewMode::FinalLighting:
        if(GetEngineSettings().renderDebug.FogEnabled)
            GEngine->renderer->GetTextureRendered = postBufferA->GetTexture(0);
        else
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
        break;

    case RenderViewMode::AO:
        GEngine->renderer->GetTextureRendered = ssaoBlurBuffer->GetTexture(0);
        break;

    case RenderViewMode::Fog: 
        break;
    } 

    // POST PROCESSING -------------------------------------------------------------------------------------- 
    auto fboFinale = postBufferB;
    if (GetEngineSettings().renderDebug.postProcess)
        fboFinale = postProcess->Execute(scene, *postBufferB, *postBufferA);  

    //Finish, to next fase
    if (GetEngineSettings().renderDebug.GammaHDR_correct) {
        fboFinale->Bind();
        postProcess->CorrectSpaceColor();
        GEngine->renderer->GetTextureRendered = fboFinale->GetTexture(0);
    }
      
    postProcess->Finish();  
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
        
        postBufferA->Resize(w, h);
        postBufferB->Resize(w, h);

        lightingBuffer->Resize(w, h);
         
        ssaoBuffer->Resize(w/2, h/2);
        ssaoBlurBuffer->Resize(w/2, h/2); 



        GEngine->renderer->width = w;
        GEngine->renderer->height = h;
    }
} 
  
  
void PBRPipeline::Shutdown() {
    if (gBuffer) { delete gBuffer; gBuffer = nullptr; }
    if (lightingBuffer) { delete lightingBuffer; lightingBuffer = nullptr; }

    if (postBufferA) { delete postBufferA; postBufferA = nullptr; }
    if (postBufferB) { delete postBufferB; postBufferB = nullptr; }

    if (ssaoBuffer) { delete ssaoBuffer; ssaoBuffer = nullptr; }
    if (ssaoBlurBuffer) { delete ssaoBlurBuffer; ssaoBlurBuffer = nullptr; } 
    IBLManager::Destroy(ibl);
}
 








/*
* opcionalmente pode usar ping pong para revesar os buffers d etextura nos fbos para sempre q escrever em um, troca para outro buffer
* para escrever, enquanto usa oq foie scrito anteriormente trocando seus ponteiros
*
// Frame atual
BindFBO(fboB);
glBindTexture(GL_TEXTURE_2D, texA);
Draw();

// Próximo frame
std::swap(texA, texB);
std::swap(fboA, fboB);
*/