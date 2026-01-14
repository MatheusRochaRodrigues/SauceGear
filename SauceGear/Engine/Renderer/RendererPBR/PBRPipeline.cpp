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

    framebuffer = new Framebuffer(width, height, { {FramebufferTextureType::ColorRGB} }, true);
    GEngine->renderer->frameScreen = framebuffer;

    gBuffer = new Framebuffer(width, height, {
        {FramebufferTextureType::Position},            // 0
        {FramebufferTextureType::Normal},              // 1
        {FramebufferTextureType::Albedo},              // 2 (RGB = baseColor, A = ?)
        {FramebufferTextureType::MetallicRoughnessAO}, // 3 (R = metallic,G = roughness,B = ao, A = ?)
    }, true);

    lightingBuffer = nullptr; // opcional

    std::cout << "init2" << std::endl;
    ibl = IBLManager::EnsureIBL(currentHDR, cacheDir );
    std::cout << "init3" << std::endl;
     
    postProcess = new PostProcess();
}

void PBRPipeline::Shutdown() { 
    if (gBuffer) { delete gBuffer; gBuffer = nullptr; }
    if (framebuffer) { delete framebuffer; framebuffer = nullptr; }
    if (lightingBuffer) { delete lightingBuffer; lightingBuffer = nullptr; }
    //if (iblFBO) glDeleteFramebuffers(1, &iblFBO);
    //if (iblRBO) glDeleteRenderbuffers(1, &iblRBO);
    IBLManager::Destroy(ibl);
}

void PBRPipeline::HandleFBOs() {
    if (!framebuffer || !gBuffer) Init();
    if (framebuffer->GetWidth() != gBuffer->GetWidth() || framebuffer->GetHeight() != gBuffer->GetHeight())
        gBuffer->Resize(framebuffer->GetWidth(), framebuffer->GetHeight());
} 
  
 

 