#pragma once
#include "../../../Graphics/Framebuffer.h"    
#include "../Bindings/GBufferBinder.h" 
#include "../Bindings/IBLBinder.h"  
#include "../../../Graphics/FullscreenQuad.h"
#include "../../../Resources/Primitives/Primitive.h"
#include "../../../Assets/MeshAsset.h"
#include "../../../Instancing/MeshInstance.h"
#include "../../LightPass/LightPass.h"
#include "../../../Graphics/Renderer.h"

using Scene = SceneECS;

struct LightInstanceData {
    glm::vec3 position;
    float radius;
    GLuint iD;
};

class DeferredLightingPass {
public:
    DeferredLightingPass(Shader* iblAmbient, Shader* dirLight, Shader* pointLight) : 
        iblAmbient(iblAmbient), dirLight(dirLight), pointLight(pointLight) { 
        //=================================== bindings fixos
        iblAmbient->use();
        iblAmbient->setInt("gPosition", 0);
        iblAmbient->setInt("gNormal", 1);
        iblAmbient->setInt("gAlbedo", 2);
        iblAmbient->setInt("gMRA", 3);
        iblAmbient->setInt("irradianceMap", 4);
        iblAmbient->setInt("prefilterMap", 5);
        iblAmbient->setInt("brdfLUT", 6);
         
        // Lighting shaders usam samplers fixos:
        dirLight->use();
        dirLight->setInt("gPosition", 0);
        dirLight->setInt("gNormal", 1);
        dirLight->setInt("gAlbedo", 2);
        dirLight->setInt("gMRAO", 3);

        pointLight->use();
        pointLight->setInt("gPosition", 0);
        pointLight->setInt("gNormal", 1);
        pointLight->setInt("gAlbedo", 2);
        pointLight->setInt("gMRAO", 3);
        pointLight->setInt("pointShadows[0]", 7); // base TMU; LightPass cuida dos offsets 


        sphereMesh = std::make_shared<MeshInstance>(PrimitiveMesh::CreateSphere2RenderingLight());
    }

    void Execute(
        Scene& scene,
        Framebuffer& target,
        Framebuffer& gbuffer, 
        IBLSet& ibl)
    {
        target.Bind();
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        // AMBIENT
        glDisable(GL_BLEND);
        iblAmbient->use();
        iblAmbient->setVec3("viewPos", GEngine->mainCamera->GetPosition());
        GBufferBinder::Bind(gbuffer);
        IBLBinder::Bind(ibl);
        RenderQuad();


        //--------------------------------------------------------------------------------------------------------------------------------
        // ------------------- SUN -------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------
        // ** ADDITIVE LIGHTS **
        glEnable(GL_BLEND); glBlendEquation(GL_FUNC_ADD); glBlendFunc(GL_ONE, GL_ONE);
        glEnable(GL_CULL_FACE); glCullFace(GL_BACK);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        //(fullscreen) RENDERIZAÇAO
        if (LightPass::SetSunToShader(dirLight)) {
            GBufferBinder::Bind(gbuffer);
            dirLight->setVec3("camPos", GEngine->mainCamera->GetPosition());
            RenderQuad();
        }
         

        //--------------------------------------------------------------------------------------------------------------------------------
        // ------------------ POINTS -----------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------
        // **  point lights (volume) **

        //aditivo via esferas instanciadas** 
        pointLight->use();
        pointLight->setVec3("viewPos", GEngine->mainCamera->GetPosition());
        pointLight->setVec2("screenSize", glm::vec2(GEngine->renderer->frameScreen->GetWidth(), GEngine->renderer->frameScreen->GetHeight()));
        pointLight->setFloat("far_plane", 25); // adapte ao seu sistema 
        GBufferBinder::Bind(gbuffer); 

        // UBO de luzes já está bound no binding=1 (igual seu shader antigo)
        LightPass::SetPointsToShader(pointLight, 0);

        // instâncias
        std::vector<LightInstanceData> instanceData;
        for (auto e : LightPass::lightInActive.point) {
            auto& light = GEngine->scene->GetComponent<LightComponent>(e);
            auto& trans = GEngine->scene->GetComponent<TransformComponent>(e);
            // then calculate radius of light volume/sphere
            const float maxBrightness = std::fmaxf(std::fmaxf(light.color.r, light.color.g), light.color.b);
            //Atenuation inverse quadratic
            const float rangeLight = 0.01;  //0.001; 
            float radius = std::sqrt(maxBrightness / rangeLight);
            instanceData.push_back({ trans.position, radius, light.depthMap });
        }

        sphereMesh->SetInstanceData(
            instanceData.data(),
            instanceData.size() * sizeof(LightInstanceData),
            {
                { 10, 3 },  // posição (vec3)
                { 11, 1 },  // raio    (float)
                { 12, 1 }   // indice  (float)
            }
        );

        // Renderiza as esferas instanciadas, uma para cada luz pontual
        sphereMesh->DrawInstanced(instanceData.size());

        //End  
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE); 
        glEnable(GL_DEPTH_TEST);      glDepthMask(GL_TRUE);         // não escreve no depth  
    }
     
private:
    Shader* iblAmbient;
    Shader* dirLight;
    Shader* pointLight;

    std::shared_ptr<MeshInstance> sphereMesh = nullptr;
};
