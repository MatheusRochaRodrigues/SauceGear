#pragma once
#include <glad/glad.h>
#include "../../../../Graphics/Shader.h"
#include "../../../../Graphics/Texture.h"
#include "../../../../Graphics/Framebuffer.h"
#include "../../../../Graphics/FullscreenQuad.h"
#include "../../../../Core/EngineContext.h"
#include "../../../../Platform/Window.h"
#include "../../../../Core/Camera.h" 
#include "../../../RenderDebugSettings.h"

class SSAOPass {
public:
    SSAOPass(Shader* shader, Shader* shaderB) : shader(shader), shaderPPBlur(shaderB) {
        GenerateKernel();
        GenerateNoise(); 
    } 

    void ExecuteOcclusion(Framebuffer& gbuffer, Framebuffer& ssao) {
        ssao.Bind();
        glClear(GL_COLOR_BUFFER_BIT);

        shader->use();

        shader->setMat4("projection", GEngine->mainCamera->GetProjectionMatrix());
        shader->setMat4("view", GEngine->mainCamera->GetViewMatrix());

        shader->setInt("gPosition", 0);
        shader->setInt("gNormal", 1); 
        shader->setInt("texNoise", 2);
        
        auto& eS = GetEngineSettings().renderDebug;
        shader->setInt("kernelSize", eS.sKernelSize);
        shader->setFloat("radius", eS.sRadius);
        shader->setFloat("bias", eS.sBias);
        shader->setFloat("power", eS.Power);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gbuffer.GetTexture(0));        // Position

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gbuffer.GetTexture(2));        // Normal

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture->ID);        // texNoise             noiseTexture->Bind(2);  
                      
        const unsigned int width = GEngine->window->GetWidth();
        const unsigned int height = GEngine->window->GetHeight();
        shader->setVec2("noiseScale", glm::vec2(ssao.GetWidth() / 4.0f, ssao.GetHeight() / 4.0f));
         
        for (int i = 0; i < kernel.size(); ++i)
            shader->setVec3("samples[" + std::to_string(i) + "]", kernel[i]);

        RenderQuad();     //DrawFullscreenQuad                  occlusion
        ssao.Unbind();
    }

    void BlurPostProcess(Framebuffer& ssao, Framebuffer& out) {
        out.Bind();
        glClear(GL_COLOR_BUFFER_BIT);

        shaderPPBlur->use();
        shaderPPBlur->setInt("ssaoInput", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssao.GetTexture(0));        // Normal
        RenderQuad();

        out.Unbind();
    }

    void Execute(Framebuffer& gbuffer, Framebuffer& ssao, Framebuffer& out) {
        glViewport(0, 0, ssao.GetWidth(), ssao.GetHeight());
        ExecuteOcclusion(gbuffer, ssao),

        //Blur PostProcess
        BlurPostProcess(ssao, out);

        // CUIDADO AQ, pois n temos certeza se essa viwport pode nao representar a real resolução da janela, observe bem isso
        GEngine->window->SetWindowViewport0();
    };
     
private:
    Shader* shader;
    Shader* shaderPPBlur;
    std::vector<glm::vec3> kernel;
    Texture* noiseTexture;

    unsigned int iSamples = 64; //32 /*Default 64*/     //how much Samples

    void GenerateKernel();
    void GenerateNoise();
};
