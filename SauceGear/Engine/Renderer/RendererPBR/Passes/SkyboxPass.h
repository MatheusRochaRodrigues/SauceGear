#pragma once
#include "../../../Graphics/Framebuffer.h"    
#include "../../../Graphics/FullscreenQuad.h"
#include "../../../ECS/Systems/DayNightSystem.h"
#include "../../LightPass/LightPass.h" 
#include "../../../Core/EngineContext.h"
#include "../../../Core/Time.h"
#include "../../../Core/Camera.h"
#include "../../../Instancing/MeshInstance.h" 
#include "../../../Resources/Primitives/Primitive.h"

class SkyboxPass {
public:
    SkyboxPass(Shader* shader) : shader(shader) {
        sphereMesh = std::make_shared<MeshInstance>(PrimitiveMesh::CreateSphere2RenderingLight());
         
        // Shaders defaults
        shader->use();
        shader->setInt("environmentMap", 0);
        
        skybox = LoadCubemap2TEX();
    }

    void Execute(Camera& cam, GLuint cubemap, Framebuffer& target)
    { 
        if (cubemap == 0) cubemap = skybox;

        target.Bind();
        DrawSkybox(cam, cubemap);
        //RenderDebugSun();
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
        if (LightPass::currentSun == INVALID_ENTITY) return;

        auto& sunTransform = GEngine->scene->GetComponent<TransformComponent>(LightPass::currentSun);
        auto& sunLight = GEngine->scene->GetComponent<LightComponent>(LightPass::currentSun);

        glm::vec3 sunDir = sunTransform.GetForwardDirection();
        glm::vec3 camPos = GEngine->mainCamera->GetPosition();
        glm::vec3 sunPos = camPos + sunDir * 80.0f; // sol "longe" da câmera

        // cria shader billboard se não existir
        static Shader* sunBillboardShader = nullptr;
        if (!sunBillboardShader)
            sunBillboardShader = new Shader("Envirolnment/SunBillboard.vs", "Envirolnment/SunBillboard.fs");

        // quad simples para billboard (2 triângulos)
        static std::shared_ptr<MeshInstance> quadMesh = nullptr;
        if (!quadMesh) {
            std::vector<Vertex> verts(4);
            verts[0].Position = { -1.0f, -1.0f, 0.0f };
            verts[1].Position = { 1.0f, -1.0f, 0.0f };
            verts[2].Position = { -1.0f,  1.0f, 0.0f };
            verts[3].Position = { 1.0f,  1.0f, 0.0f };

            std::vector<uint32_t> indices = { 0,1,2, 2,1,3 };

            auto quadAsset = std::make_shared<MeshAsset>();
            quadAsset->SetData(std::move(verts), std::move(indices));
            quadAsset->name = "SunQuad";

            quadMesh = std::make_shared<MeshInstance>(quadAsset);
        }

        // pass uniforms
        sunBillboardShader->use();
        sunBillboardShader->setVec3("sunPos", sunPos);
        sunBillboardShader->setMat4("view", GEngine->mainCamera->GetViewMatrix());
        sunBillboardShader->setMat4("projection", GEngine->mainCamera->GetProjectionMatrix());
        sunBillboardShader->setFloat("size", 20.0f); // tamanho do sol
        sunBillboardShader->setVec3("color", sunLight.color);
        sunBillboardShader->setFloat("time", GEngine->time->GetTime());

        sunBillboardShader->setFloat("intensity", sunLight.intensityBillboard);

        /*
        // blending aditivo
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
        glDepthMask(GL_FALSE);      //glDisable(GL_DEPTH_TEST);

        quadMesh->Draw();

        glDepthMask(GL_TRUE);       //glEnable(GL_DEPTH_TEST); 
        glDisable(GL_BLEND);
        */


        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
        
        glEnable(GL_DEPTH_TEST); //  Usa depth buffer existente
        glDepthFunc(GL_LESS); 
        // NÃO escreve depth
        glDepthMask(GL_FALSE);

        quadMesh->Draw();

        // restaura
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

    }


    static inline std::string path = "Assets/HDR/sky_47_2k (1)/sky_47_cubemap_2k";
    GLuint skybox;
private:
    Shader* shader;
    std::shared_ptr<MeshInstance> sphereMesh = nullptr;

    static GLuint LoadCubemap2TEX();
};
