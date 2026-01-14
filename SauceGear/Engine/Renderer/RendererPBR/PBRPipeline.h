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

using Scene = SceneECS; 

class PBRPipeline : public IRenderPipeline {
public:
    void Init() override;
     
    void Render(Scene& scene) override {   
        //=============================================TimeLine============================================================
        HandleFBOs();                       //ibl = DayNightSystem::GetSkyboxFront();

        //Render Shadow
        lightPass->Update();
        // GBUFFER + STENCIL
        geometryPass->Execute(scene, *gBuffer);                  
        // DEFERRED LIGHTING
        lightingPass->Execute(scene, *framebuffer, *gBuffer, ibl);
        // OUTLINE PASS   
        
        // FORWARD  
        forwardPass->Execute(scene, *gBuffer, *framebuffer);
        // SKYBOX 
        skyboxPass->Execute(*GEngine->mainCamera, ibl.envCubemap);  //shaders.skybox
         
        //postProcess->Update();
        
        //========================================BUFFER RENDER============================================================
        GEngine->renderer->GetTextureRendered = framebuffer->GetTexture(0); 
        //auto& light = GEngine->scene->GetComponent<LightComponent>(LightSystem::currentSun); GEngine->renderer->GetTextureRendered = light.depthMap;
        //GEngine->renderer->GetTextureRendered = gBuffer->GetTexture(2);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Shutdown() override;

    // util
    std::string currentHDR = "Engine/Resources/Textures/hdr/spruit_sunrise_4k.hdr";
    std::string cacheDir = "Engine/Resources/Cache/IBL";

    Framebuffer* framebuffer = nullptr; // final color + depth (default: true depth) 
private:
    LightPass*               lightPass;
    GeometryPass*            geometryPass;
    DeferredLightingPass*    lightingPass;
    ForwardPass*             forwardPass;
    SkyboxPass*              skyboxPass;

    //Post process
    PostProcess*             postProcess;

    Framebuffer* gBuffer = nullptr;         // Position/Normal/Albedo/MetalRoughAO (+depth)                 gBufferFBO
    Framebuffer* lightingBuffer = nullptr; // opcional se quiser acumular separado (aqui vou direto no framebuffer final)
     
    // IBL 
    IBLSet     ibl {};                                                          //GLuint     iblFBO = 0, iblRBO = 0; 

    // passes
    void HandleFBOs();  
};
