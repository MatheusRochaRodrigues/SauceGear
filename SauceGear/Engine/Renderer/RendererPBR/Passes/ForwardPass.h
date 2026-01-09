#pragma once 
#include "../../../Graphics/Framebuffer.h"   
#include "../../../Graphics/Renderer.h"

using Scene = SceneECS;

class ForwardPass {
public:
    void Execute(Scene& scene, Framebuffer& gbuffer, Framebuffer& framebuffer)
    {
        // (opcional) transparências/partículas usando PBR forward com IBL:
        // copiar depth do gbuffer -> framebuffer se quiser ordenar contra opacos 

        //glEnable(GL_DEPTH_TEST);
        //glDepthFunc(GL_LESS);

        int SCR_WIDTH = gbuffer.GetWidth();
        int SCR_HEIGHT = gbuffer.GetHeight();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer.GetID());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.GetID()); // write to default framebuffer
        glBlitFramebuffer(
            0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );
        //framebuffer->Bind();                    // glBindFramebuffer(GL_FRAMEBUFFER, ); 

        // now render light cubes as before
        auto camera = GEngine->mainCamera;
        auto view = camera->GetViewMatrix();
        auto proj = camera->GetProjectionMatrix();


        /*
        for (auto e : scene.View<MeshRenderer, Transform>()) {
            auto& mr = scene.Get<MeshRenderer>(e);
            mr.Draw(RenderPassType::Forward);
        }
        */


        // ... desenhe transparentes com shader PBR forward e BindIBLTo() 
    }
};
