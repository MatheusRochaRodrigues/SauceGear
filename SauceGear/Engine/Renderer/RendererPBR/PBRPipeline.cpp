#include "PBRPipeline.h" 
  
void PBRPipeline::Init() {
    std::cout << "init" << std::endl; 
    lightPass = new LightPass();

    geometryPass = new GeometryPass(&ShaderLibrary::Get("PBR_GBuffer"));
    lightingPass = new DeferredLightingPass(
        &ShaderLibrary::Get("PBR_IBL"),
        &ShaderLibrary::Get("PBR_Directional"),
        &ShaderLibrary::Get("PBR_Point")
    ); 
    skyboxPass =  new SkyboxPass(&ShaderLibrary::Get("Skybox"));

    const unsigned int width = GEngine->window->GetWidth();
    const unsigned int height = GEngine->window->GetHeight();

    lightingBuffer = new Framebuffer(width, height, { {FramebufferTextureType::HDR} }, true); //ColorRGB  
    GEngine->renderer->frameScreen = lightingBuffer;

    gBuffer = new Framebuffer(width, height, {
        {FramebufferTextureType::Position},            // 0
        {FramebufferTextureType::Albedo},  //HDR            // 1 (RGB = baseColor, A = ?)            //Legacy ::Albedo
        {FramebufferTextureType::Normal},              // 2
        {FramebufferTextureType::MetallicRoughnessAO}, // 3 (R = metallic,G = roughness,B = ao, A = ?)
    }, true);  
     
    postProcess = new PostProcess();
    ppBuffer = new Framebuffer(width, height, { {FramebufferTextureType::HDR} }, true);
     
    std::cout << "initIBL" << std::endl;
    ibl = IBLManager::EnsureIBL(currentHDR, cacheDir);
    std::cout << "finishIBL" << std::endl;
}

void PBRPipeline::Shutdown() { 
    if (gBuffer) { delete gBuffer; gBuffer = nullptr; }
    if (lightingBuffer) { delete lightingBuffer; lightingBuffer = nullptr; }
    if (lightingBuffer) { delete lightingBuffer; lightingBuffer = nullptr; }
    //if (iblFBO) glDeleteFramebuffers(1, &iblFBO);     //if (iblRBO) glDeleteRenderbuffers(1, &iblRBO);
    IBLManager::Destroy(ibl);
}

void PBRPipeline::HandleFBOs() {
    if (!lightingBuffer || !gBuffer) Init();
    if (lightingBuffer->GetWidth() != gBuffer->GetWidth() || lightingBuffer->GetHeight() != gBuffer->GetHeight())
        gBuffer->Resize(lightingBuffer->GetWidth(), lightingBuffer->GetHeight());
} 
  
 

 