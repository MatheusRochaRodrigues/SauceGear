#pragma once
#include "../IRenderPipeline.h"
#include "../../ECS/Systems/LightSystem.h"
#include "../../Framebuffer.h"
#include "../../../Resources/Primitive.h"
#include "IBLManager.h"
#include "PBRShaders.h"

using Scene = SceneECS;

class PBRPipeline : public IRenderPipeline {
public:
    void Init() override;
     
    void Render(Scene& scene) override {
        std::cout << "f9" << std::endl;
        HandleFBOs();
        std::cout << "f19" << std::endl;
        GeometryPass(scene);
        std::cout << "f1119" << std::endl;
        LightingPass(scene);
        ForwardPass(scene);
        // 
        // 3) Skybox (no final pra não interferir na iluminação) 
        //DrawSkybox(); 
        // saída final
        GEngine->renderer->GetTextureRendered = framebuffer->GetTexture(0); 
        //auto& light = GEngine->scene->GetComponent<LightComponent>(LightSystem::currentSun); GEngine->renderer->GetTextureRendered = light.depthMap;
        //GEngine->renderer->GetTextureRendered = gBuffer->GetTexture(1);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Shutdown() override;

private:
    Framebuffer* framebuffer = nullptr; // final color + depth (default: true depth)
    Framebuffer* gBuffer = nullptr; // Position/Normal/Albedo/MetalRoughAO (+depth)
    Framebuffer* lightingBuffer = nullptr; // opcional se quiser acumular separado (aqui vou direto no framebuffer final)

    Mesh* sphereMesh = nullptr;

    // IBL
    PBRShaders shaders;
    IBLSet     ibl {};
    GLuint     iblFBO = 0, iblRBO = 0; 

    // passes
    void HandleFBOs();
    void GeometryPass(Scene& scene);
    void LightingPass(Scene& scene);
    void ForwardPass(Scene& scene);

    // binders
    void BindGBufferTo(Shader* s);
    void BindIBLTo(Shader* s);

    // background skybox
    void DrawSkybox();

    // util
    std::string currentHDR = "Resources/Textures/hdr/tst/dikhololo_night_4k.hdr";
    //std::string currentHDR = "Resources/Textures/hdr/tst/Kloppenheim (1).hdr";
    std::string cacheDir = "Resources/Cache/IBL";
};
