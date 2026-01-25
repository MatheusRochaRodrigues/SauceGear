#pragma once
#include "../../ECS/Systems/DebugRenderer.h"
#include "../../Core/EngineContext.h" 

#include "../../ECS/Components/OutlineComponent.h"
#include "../../ECS/Components/MeshRenderer.h"
#include "../../ECS/Components/TransformComponent.h"

class OutlinePass {
public:
    OutlinePass(Shader* outlineShader) : outlineShader(outlineShader) {};

    void Execute(Scene& scene, Framebuffer& target) {
        glBindFramebuffer(GL_FRAMEBUFFER, target.GetID());
         
        // 1 desabilitar depth gera um efeito muito interessante que permite vc ver a sinueta do objeto(style Fill) quando sobreposto por outros objetos
        // glDisable(GL_DEPTH_TEST); 
        // 2
        glEnable(GL_DEPTH_TEST);    glDepthFunc(GL_LEQUAL);

        glEnable(GL_STENCIL_TEST); 
        // só desenha onde stencil != 1
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);    // Desabilita a escrita, usando mascara AND com bit 0 

        outlineShader->use();
        outlineShader->setMat4("view", GEngine->mainCamera->GetViewMatrix());
        outlineShader->setMat4("projection", GEngine->mainCamera->GetProjectionMatrix());
          
        for (auto e : scene.GetEntitiesWith<OutlineComponent, MeshRenderer, TransformComponent>()) {
            auto& o = scene.GetComponent<OutlineComponent>(e);
            if (!o.enabled) continue;

            auto& tr = scene.GetComponent<TransformComponent>(e);
            auto& mr = scene.GetComponent<MeshRenderer>(e);

            outlineShader->setMat4("model", tr.GetMatrix());
            outlineShader->setVec3("outlineColor", o.color);
            outlineShader->setFloat("outlineScale", o.thickness);

            mr.DrawMesh();
        } 
        glStencilMask(0xFF);
        glDisable(GL_STENCIL_TEST);

        //glEnable(GL_DEPTH_TEST);
    } 
     
private:
    Shader* outlineShader;
};
 






 