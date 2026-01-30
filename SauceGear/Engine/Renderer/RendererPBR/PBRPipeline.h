#pragma once
#include "../IRenderPipeline.h" 
#include "../../Graphics/Framebuffer.h" 
#include "IBL/IBLManager.h"
#include "../../Core/EngineContext.h"

using Scene = SceneECS; 
 
class LightPass;
class GeometryPass;
class DeferredLightingPass;
class ForwardPass;
class SkyboxPass;
class PostProcess;
class OutlinePass;

class SharedDepthStencil;
 
class SSAOPass;
//class SSAOBlurPass;

class FogPass;

class PBRPipeline : public IRenderPipeline {
public:
    void Init() override;
     
    void Render(Scene& scene) override;

    void Shutdown() override;

    // util
    std::string currentHDR = "Engine/Resources/Textures/hdr/spruit_sunrise_4k.hdr";
    //std::string currentHDR = "Assets/HDR/sky_47_2k (1)/sky_47_2k.png";
    std::string cacheDir = "Engine/Resources/Cache/IBL";

private:
    LightPass*               lightPass;
    GeometryPass*            geometryPass;
    DeferredLightingPass*    lightingPass;
    ForwardPass*             forwardPass;
    SkyboxPass*              skyboxPass;  
    PostProcess*             postProcess;   
    OutlinePass*             outlinePass;
    //SSAO
    SSAOPass*                ssaoPass;

    FogPass*                 fogPass;

    //FBOs
    SharedDepthStencil* sharedDepthStencil; //Depth Shared

    Framebuffer* gBuffer = nullptr;         // Position/Normal/Albedo/MetalRoughAO (+depth)                 gBufferFBO
    Framebuffer* lightingBuffer = nullptr;     // final color + depth (default: true depth)        //tambem pode ser conhecido como lightingBuffer
    
    Framebuffer* postBufferA = nullptr;  // opcional se quiser acumular separado (aqui vou direto no lightingBuffer final)
    Framebuffer* postBufferB = nullptr;  // opcional se quiser acumular separado (aqui vou direto no lightingBuffer final)
    
    Framebuffer* ssaoBuffer     = nullptr;
    Framebuffer* ssaoBlurBuffer = nullptr;

    // IBL 
    IBLSet     ibl {};                                                          //GLuint     iblFBO = 0, iblRBO = 0; 

    // passes
    void HandleFBOs();  
};
