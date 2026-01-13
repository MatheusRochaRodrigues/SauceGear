#pragma once
#include "../../../Graphics/Framebuffer.h"    
#include "../../../Graphics/FullscreenQuad.h"
#include "../../../ECS/Systems/DayNightSystem.h"
#include "../../../Graphics/PrimitiveMesh.h"
#include "../../LightPass/LightPass.h"
  
class SkyboxPass {
public:
    SkyboxPass(Shader* shader) : shader(shader) {
        sphereMesh = std::make_shared<MeshInstance>(PrimitiveMesh::CreateSphere2RenderingLight());
         
        // Shaders defaults
        shader->use();
        shader->setInt("environmentMap", 0);
    }

    void Execute(Camera& cam, GLuint cubemap)
    { 
        DrawSkybox(cam, cubemap);
        RenderDebugSun();
    }

    void DrawSkybox(Camera& cam, GLuint cubemap) {
        glDepthFunc(GL_LEQUAL);
        shader->use();
        shader->setMat4("view", cam.GetViewMatrix());
        shader->setMat4("projection", cam.GetProjectionMatrix());

        glActiveTexture(GL_TEXTURE0);

        // Nunca chame StartCubemapJob() dentro do draw. Faça no Update 
        // Bind seguro
        //glBindTexture(GL_TEXTURE_CUBE_MAP, DayNightSystem::GetSkyboxFront().envCubemap);  //glBindTexture(GL_TEXTURE_CUBE_MAP, ibl.prefilter);

        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

        RenderCube();
        glDepthFunc(GL_LESS);  
    }

    void RenderDebugSun() {
        // pega luz e transform do sol
        if (LightPass::currentSun == INVALID_ENTITY) return;

        auto& sunTransform = GEngine->scene->GetComponent<TransformComponent>(LightPass::currentSun);
        auto& sunLight = GEngine->scene->GetComponent<LightComponent>(LightPass::currentSun);

        // direção e posição
        glm::vec3 sunDir = sunTransform.GetForwardDirection();
        glm::vec3 camPos = GEngine->mainCamera->GetPosition();
        glm::vec3 sunPos = camPos + sunDir * 80.0f; // coloca “longe” da câmera
          
        // cria shader se ainda não existir
        static Shader* debugSunShader = nullptr;
        if (!debugSunShader) debugSunShader = new Shader("Envirolnment/Sun.vs", "Envirolnment/Sun.fs");

        debugSunShader->use(); 
        glm::mat4 model = glm::translate(glm::mat4(1.0f), sunPos);
        model = glm::scale(model, glm::vec3(20.0f)); // tamanho da esfera

        debugSunShader->setMat4("model", model);
        debugSunShader->setMat4("view", GEngine->mainCamera->GetViewMatrix());
        debugSunShader->setMat4("projection", GEngine->mainCamera->GetProjectionMatrix()); 
        debugSunShader->setVec3("color", glm::vec3(0.5f, 0, 0.5f));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        sphereMesh->Draw();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

private:
    Shader* shader;
    std::shared_ptr<MeshInstance> sphereMesh = nullptr;
};
