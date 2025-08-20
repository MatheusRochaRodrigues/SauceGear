#pragma once
#include "../IRenderPipeline.h" 
#include "../../Framebuffer.h"
#include "../../../Resources/Primitive.h" 
#include "../Graphics/Renderer.h"

class BlinnPhongPipeline : public IRenderPipeline {
public:
    void Init() override {
        unsigned int width = GEngine->window->GetWidth();
        unsigned int height = GEngine->window->GetHeight();

        framebuffer = new Framebuffer(width, height, { {FramebufferTextureType::ColorRGB} }, true);
        GEngine->renderer->frameScreen = framebuffer;

        gBuffer = new Framebuffer(width, height, {
            {FramebufferTextureType::Position},
            {FramebufferTextureType::Normal},
            {FramebufferTextureType::ColorRGBA}
        }, true);
        
        lightingBuffer = new Framebuffer(width, height, {
            {FramebufferTextureType::ColorRGB}
        }, false);
        
        sphereMesh = PrimitiveMesh::CreateSphere2RenderingLight(); // cria esfera uma vez
    }

    void Render(SceneECS& scene) override {
        HandleFBOs();

        //render scene
        
        // ===== Geometry Pass =====
        GeometryPass(scene);

        // ===== Lighting Pass =====
        LightingPass(scene);

        // ===== Forward Pass =====
        ForwardPass(scene);

         
        //--------- to end fbo
        //holding data texture for post processing
        //GEngine->renderer->GetTextureRendered = gBuffer->GetTexture(2);
        GEngine->renderer->GetTextureRendered = framebuffer->GetTexture(0);
        //GEngine->renderer->GetTextureRendered = GEngine->scene->GetComponent<LightComponent>(LightSystem::currentSun).depthMap;
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default 
    }

    void Shutdown() override {
        // liberar recursos
    }
private:
    Framebuffer* framebuffer = nullptr;
    Framebuffer* gBuffer = nullptr;
    Framebuffer* lightingBuffer = nullptr;

    Mesh* sphereMesh = nullptr;

    void HandleFBOs();

    void GeometryPass(SceneECS& scene);
    
    void LightingPass(SceneECS& scene);
    void handle_GBufferShader(Shader* shader); 

    void ForwardPass(SceneECS& scene);
};
 