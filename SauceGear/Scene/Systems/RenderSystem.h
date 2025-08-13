#pragma once

#include "../../Core/EngineContext.h"
#include "../Components/Transform.h"
#include "../Components/MeshRenderer.h"
#include "../Components/Material.h"
#include "../Systems/LightSystem.h"
//#include "../Resources/PrimitiveMesh.h"
//#include "../LightingUtils.h"

class RenderSystem : public System {
public:
    void init() {
        unsigned int width = GEngine->window->GetWidth();
        unsigned int height = GEngine->window->GetHeight();
        framebuffer = new Framebuffer(width, height, { {FramebufferTextureType::ColorRGB} }, true);
        GEngine->renderer->frameScreen = framebuffer;

        //others
        gBuffer = new Framebuffer(width, height, {
            {FramebufferTextureType::Position},
            {FramebufferTextureType::Normal},
            {FramebufferTextureType::ColorRGBA}
        }, true);

        lightingBuffer = new Framebuffer(width, height, {
            {FramebufferTextureType::ColorRGB}
        }, false);


        //glGenBuffers(1, &instanceVBO);
        sphereMesh = PrimitiveMesh::CreateSphere2RenderingLight(); // cria esfera uma vez
    }

    void Update(float dt) override {
        /*RenderShadowPass();       RenderMainPass();*/
        //lightSystem->ShadowPass();  // antes de desenhar o mundo

        try { 
            //--------- to start fbo
            handleFBOs(); 
            //GEngine->window->SetWindowViewport0();
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->GetID());
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
            glEnable(GL_DEPTH_TEST);


            //RenderMainPass();          
            
            //render scene
            GeometryPass();
            LightingPass();
            ForwardPass();

            //--------- to end fbo
            //holding data texture for post processing
            //GEngine->renderer->GetTextureRendered = gBuffer->GetTexture(0);
            GEngine->renderer->GetTextureRendered = framebuffer->GetTexture(0);
            //GEngine->renderer->GetTextureRendered = GEngine->scene->GetComponent<LightComponent>(LightSystem::currentSun).depthMap;
            glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default 

        } catch (const std::exception& e) {
            std::cerr << "[EXCEÇÃO - RenderSystem] " << e.what() << "\n";
        }

        //std::cout << "FPS: " << 1.0f / Time::GetDeltaTime() << std::endl;
        //std::cout << "FPS: " << 1.0f / dt << std::endl;
    } 
       
private:  
    Framebuffer* framebuffer = nullptr;
     
    Framebuffer* gBuffer = nullptr;
    Framebuffer* lightingBuffer = nullptr;

    Mesh* sphereMesh = nullptr;
    //GLuint instanceVBO;
    
    void handleFBOs();
    void handle_GBufferShader(Shader* shader);

    void GeometryPass();
    void LightingPass();
    void ForwardPass();

    //---------------------------------Render Pass Main
    void RenderMainPass();


};


struct LightInstanceData {
    glm::vec3 position;
    float radius;
    GLuint iD;
};
 