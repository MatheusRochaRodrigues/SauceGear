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

using Scene = SceneECS; 

class PBRPipeline : public IRenderPipeline {
public:
    void Init() override;
     
    void Render(Scene& scene) override {   
        HandleFBOs();
        //ibl = DayNightSystem::GetSkyboxFront();

        // GBUFFER + STENCIL
        geometryPass->Execute(scene, *gBuffer);                  //shaders.gbuffer
        // DEFERRED LIGHTING
        lightingPass->Execute(scene, *framebuffer, *gBuffer, ibl);
        // OUTLINE PASS  
        
        // FORWARD  
        forwardPass->Execute(scene, *gBuffer, *framebuffer);
        // SKYBOX
        skyboxPass->Execute(*GEngine->mainCamera, ibl.prefilter);  //shaders.skybox
         
           
        GEngine->renderer->GetTextureRendered = framebuffer->GetTexture(0); 
        //auto& light = GEngine->scene->GetComponent<LightComponent>(LightSystem::currentSun); GEngine->renderer->GetTextureRendered = light.depthMap;
        //GEngine->renderer->GetTextureRendered = gBuffer->GetTexture(2);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Shutdown() override;

    // util
    std::string currentHDR = "Engine/Resources/Textures/hdr/tst/dikhololo_night_4k.hdr";
    std::string cacheDir = "Engine/Resources/Cache/IBL";

    Framebuffer* framebuffer = nullptr; // final color + depth (default: true depth) 
private: 
    GeometryPass*            geometryPass;
    DeferredLightingPass*    lightingPass;
    ForwardPass*             forwardPass;
    SkyboxPass*              skyboxPass;

    Framebuffer* gBuffer = nullptr;         // Position/Normal/Albedo/MetalRoughAO (+depth)                 gBufferFBO
    Framebuffer* lightingBuffer = nullptr; // opcional se quiser acumular separado (aqui vou direto no framebuffer final)
     
    // IBL 
    IBLSet     ibl {};
    //GLuint     iblFBO = 0, iblRBO = 0; 

    // passes
    void HandleFBOs();  
};
