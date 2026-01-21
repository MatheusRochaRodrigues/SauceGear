#pragma once
#include "../IRenderPipeline.h" 
#include "../../Graphics/Framebuffer.h" 
#include "IBL/IBLManager.h"
#include "../../Materials/MaterialLibrary.h"
#include "../Shaders/ShaderLibrary.h"
#include "../../Core/EngineContext.h"
#include "../../Graphics/Renderer.h"

#include "Passes/GeometryPass.h"
#include "Passes/DeferredLightingPass.h"
#include "Passes/ForwardPass.h"
#include "Passes/SkyboxPass.h"
#include "Passes/TransparentPass.h"

#include "../LightPass/LightPass.h"
#include "../PostProcessPass/PostProcess.h"
#include "../RenderDebugSettings.h"

using Scene = SceneECS; 

class PBRPipeline : public IRenderPipeline {
public:
    void Init() override;
     
    void Render(Scene& scene) override {   
        auto& engineSettings = GetEngineSettings();
        auto& debug = engineSettings.renderDebug;
        //=============================================TimeLine============================================================
        HandleFBOs();                       //ibl = DayNightSystem::GetSkyboxFront();

        //Render Shadow
        lightPass->Update(debug.Shadow);
        // GBUFFER + STENCIL
        geometryPass->Execute(scene, *gBuffer);                  
        // DEFERRED LIGHTING
        lightingPass->Execute(scene, *lightingBuffer, *gBuffer, ibl);
        // OUTLINE PASS   
        
        // FORWARD  
        forwardPass->Execute(scene, *gBuffer, *lightingBuffer);
        // SKYBOX 
        if (GetEngineSettings().renderDebug.Skybox)
            skyboxPass->Execute(*GEngine->mainCamera, ibl.envCubemap);  //shaders.skybox
          

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
            //GEngine->renderer->GetTextureRendered = gBuffer->GetDepthTexture();
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

    void Shutdown() override;

    // util
    std::string currentHDR = "Engine/Resources/Textures/hdr/spruit_sunrise_4k.hdr";
    std::string cacheDir = "Engine/Resources/Cache/IBL";

private:
    LightPass*               lightPass;
    GeometryPass*            geometryPass;
    DeferredLightingPass*    lightingPass;
    ForwardPass*             forwardPass;
    SkyboxPass*              skyboxPass; 
    //Post process
    PostProcess*             postProcess;  

    Framebuffer* gBuffer = nullptr;         // Position/Normal/Albedo/MetalRoughAO (+depth)                 gBufferFBO
    Framebuffer* lightingBuffer = nullptr;     // final color + depth (default: true depth)        //tambem pode ser conhecido como lightingBuffer
    Framebuffer* ppBuffer = nullptr;  // opcional se quiser acumular separado (aqui vou direto no lightingBuffer final)
     
    // IBL 
    IBLSet     ibl {};                                                          //GLuint     iblFBO = 0, iblRBO = 0; 

    // passes
    void HandleFBOs();  
};
