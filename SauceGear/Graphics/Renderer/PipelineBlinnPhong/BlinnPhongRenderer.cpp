#include "BlinnPhongRenderer.h" 
#include "../../ECS/Components/TransformComponent.h"
#include "../../ECS/Components/MeshRenderer.h"
//#include "../../ECS/Components/Material.h"
#include "../../ECS/Systems/Lighting/LightSystem.h"

#include "../Graphics/FullscreenQuad.h"

void BlinnPhongPipeline::HandleFBOs() {
    if (framebuffer == nullptr || gBuffer == nullptr || lightingBuffer == nullptr) Init();

    if (framebuffer->GetWidth() != gBuffer->GetWidth() || framebuffer->GetHeight() != gBuffer->GetHeight())
        gBuffer->Resize(framebuffer->GetWidth(), framebuffer->GetHeight()); 
}


void BlinnPhongPipeline::GeometryPass(SceneECS& scene) {
    gBuffer->Bind();
    glClearColor(0.0, 0.0, 0.0, 1.0); // keep it black so it doesn't leak into g-buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glEnable(GL_DEPTH_TEST);  

    auto camera = GEngine->mainCamera;      //auto view = camera->GetViewMatrix(); //auto proj = camera->GetProjectionMatrix(); 
    auto entities = GEngine->scene->GetEntitiesWith<TransformComponent, MeshRenderer>();

    for (auto e : entities) {
        auto& trans = GEngine->scene->GetComponent<TransformComponent>(e);
        auto& meshRenderer = GEngine->scene->GetComponent<MeshRenderer>(e);

        Shader* shader = GEngine->renderer->GetGBufferShader;
        shader->setMat4("model", trans.GetMatrix());

        //for each material
        for (auto& [material, meshData] : meshRenderer.batches) {
            //for each mesh into the material
            for (auto& mesh : meshData) {
                material->Apply(shader);
                meshRenderer.DrawSubM(mesh);   // mesh.Draw();
            }
        }
    }
    gBuffer->Unbind();
}



void BlinnPhongPipeline::LightingPass(SceneECS& scene) {
    framebuffer->Bind();            //lightingBuffer->Bind();            
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                   //glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT);                           //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    //depht
    glDisable(GL_DEPTH_TEST);      // ou glDepthFunc(GL_ALWAYS)     Como o depth do volume da esfera não representa o objeto real
    //glDepthMask(GL_FALSE);         // não escreve no depth

    //mesclagem
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //====================Directional

     //glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(LightInstanceData), nullptr, GL_DYNAMIC_DRAW); 
    Shader* shader = GEngine->renderer->GetSunLightingShader;
    shader->use();
    if (LightSystem::SetSunToShader(shader) != 0) {
        handle_GBufferShader(shader);
        RenderQuad();
    }


    //================================Point===================================================== 
    ///*    // culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);


    shader = GEngine->renderer->GetLightingShader;
    shader->use();
    shader->setVec2("screenSize", glm::vec2(GEngine->renderer->frameScreen->GetWidth(), GEngine->renderer->frameScreen->GetHeight()));

    handle_GBufferShader(shader);

    // Envia informações das luzes para o shader (UBO ou outro método)
    LightSystem::SetPointsToShader(shader);

    std::vector<LightInstanceData> instanceData;
    for (auto e : LightSystem::lightInActive.point) {
        auto& light = GEngine->scene->GetComponent<LightComponent>(e);
        auto& trans = GEngine->scene->GetComponent<TransformComponent>(e);

        // update attenuation parameters and calculate radius
        const float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
        const float linear = 0.7f;
        const float quadratic = 1.8f;
        // then calculate radius of light volume/sphere
        const float maxBrightness = std::fmaxf(std::fmaxf(light.color.r, light.color.g), light.color.b);

        //Atenuation constant linear quadratic
        //const float rangeLight = (256.0f / 5.0f);
        //float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - rangeLight * maxBrightness))) / (2.0f * quadratic);
        //float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);

        //Atenuation inverse quadratic
        const float rangeLight = 0.01;
        //const float rangeLight = 0.001;
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

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);      // ou glDepthFunc(GL_ALWAYS)
    glDepthMask(GL_TRUE);         // não escreve no depth

    //*/

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);  

    //framebuffer->Unbind();   //lightingBuffer->Unbind();
}


void BlinnPhongPipeline::handle_GBufferShader(Shader* shader) {
    GLuint posTex = gBuffer->GetTextureByType(FramebufferTextureType::Position);
    GLuint normTex = gBuffer->GetTextureByType(FramebufferTextureType::Normal);
    GLuint albedoTex = gBuffer->GetTextureByType(FramebufferTextureType::ColorRGBA);

    if (posTex == 0 || normTex == 0 || albedoTex == 0) std::cout << "textura com problemas aq" << std::endl;

    // No seu shader de luz:
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, posTex);
    glUniform1i(glGetUniformLocation(shader->ID, "gPosition"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normTex);
    glUniform1i(glGetUniformLocation(shader->ID, "gNormal"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, albedoTex);
    glUniform1i(glGetUniformLocation(shader->ID, "gAlbedoSpec"), 2);

}


void BlinnPhongPipeline::ForwardPass(SceneECS& scene) {
    //int SCR_WIDTH = gBuffer->GetWidth();
    //int SCR_HEIGHT = gBuffer->GetHeight();
    //glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer->GetID());
    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer->GetID()); // write to default framebuffer
    //glBlitFramebuffer(
    //    0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST
    //);
    //framebuffer->Bind();                    // glBindFramebuffer(GL_FRAMEBUFFER, ); 

    //// now render light cubes as before
    //auto camera = GEngine->mainCamera;
    //auto view = camera->GetViewMatrix();
    //auto proj = camera->GetProjectionMatrix();

    //auto entities = GEngine->scene->GetEntitiesWith<TransformComponent, MeshRenderer, TransparentTag>();

    //for (auto e : entities) {
    //    auto& trans = GEngine->scene->GetComponent<TransformComponent>(e);
    //    auto& meshR = GEngine->scene->GetComponent<MeshRenderer>(e);
    //    if (!meshR.model) continue;

    //    for (int i = 0; i < meshR.model->GetMeshes().size(); ++i) {
    //        Material* mat = meshR.GetMaterial(i);
    //        Shader* shader = mat->shader;
    //        shader->use();
    //        shader->setMat4("model", trans.GetMatrix());
    //        shader->setMat4("view", view);
    //        shader->setMat4("projection", proj);
    //        LightSystem::SetLightsToShader(shader);
    //        mat->Bind();
    //        meshR.model->GetMeshes()[i].Draw();
    //    }
    //}

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
}