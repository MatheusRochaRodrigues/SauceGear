#include "PBRRenderer.h"
#include "../../Scene/Components/Transform.h"
#include "../../Scene/Components/MeshRenderer.h"
#include "../../Scene/Components/Material.h"
#include "../Graphics/FullscreenQuad.h"

void PBRPipeline::Init() {
    const unsigned int width = GEngine->window->GetWidth();
    const unsigned int height = GEngine->window->GetHeight();

    framebuffer = new Framebuffer(width, height, { {FramebufferTextureType::ColorRGB} }, true);
    GEngine->renderer->frameScreen = framebuffer;

    gBuffer = new Framebuffer(width, height, {
        {FramebufferTextureType::Position},            // 0
        {FramebufferTextureType::Normal},              // 1
        {FramebufferTextureType::Albedo},              // 2 (RGB = baseColor, A = ?)
        {FramebufferTextureType::MetallicRoughnessAO}, // 3 (R = metallic,G = roughness,B = ao, A = ?)
        }, true);

    lightingBuffer = nullptr; // opcional

    sphereMesh = PrimitiveMesh::CreateSphere2RenderingLight();

    // FBO p/ IBL
    glGenFramebuffers(1, &iblFBO);
    glGenRenderbuffers(1, &iblRBO); 
     
    //PBR
    // prepara IBL (com cache)
    ibl = IBLManager::EnsureIBL(currentHDR, cacheDir,
        shaders.hdrToCube, shaders.irradiance, shaders.prefilter, shaders.brdf,
        iblFBO, iblRBO);
    
    //=================================== bindings fixos
    shaders.iblAmbientShader.use();
    shaders.iblAmbientShader.setInt("gPosition", 0);
    shaders.iblAmbientShader.setInt("gNormal", 1);
    shaders.iblAmbientShader.setInt("gAlbedo", 2);
    shaders.iblAmbientShader.setInt("gMRA", 3);
    shaders.iblAmbientShader.setInt("irradianceMap", 4);
    shaders.iblAmbientShader.setInt("prefilterMap", 5);
    shaders.iblAmbientShader.setInt("brdfLUT", 6);


    // Lighting shaders usam samplers fixos:
    shaders.dirLight.use();
    shaders.dirLight.setInt("gPosition", 0);
    shaders.dirLight.setInt("gNormal", 1);
    shaders.dirLight.setInt("gAlbedo", 2);
    shaders.dirLight.setInt("gMRAO", 3);
    //It's not necessary beacuse the previous step already work about they
    //shaders.dirLight.setInt("irradianceMap", 4);        //IBL   
    //shaders.dirLight.setInt("prefilterMap", 5);         //IBL
    //shaders.dirLight.setInt("brdfLUT", 6);              //IBL

    shaders.pointLight.use();
    shaders.pointLight.setInt("gPosition", 0);
    shaders.pointLight.setInt("gNormal", 1);
    shaders.pointLight.setInt("gAlbedo", 2);
    shaders.pointLight.setInt("gMRAO", 3);
    //It's not necessary beacuse the previous step already work about they
    //shaders.pointLight.setInt("irradianceMap", 4);
    //shaders.pointLight.setInt("prefilterMap", 5);
    //shaders.pointLight.setInt("brdfLUT", 6); 
    shaders.pointLight.setInt("pointShadows[0]", 7); // base TMU; LightSystem cuida dos offsets
    //shaders.pointLight.setVec2("screenSize", glm::vec2(width, height));

    // Shaders defaults
    shaders.skybox.use(); 
    shaders.skybox.setInt("environmentMap", 0);  
}

void PBRPipeline::Shutdown() {
    if (sphereMesh) sphereMesh = nullptr;
    if (gBuffer) { delete gBuffer; gBuffer = nullptr; }
    if (framebuffer) { delete framebuffer; framebuffer = nullptr; }
    if (lightingBuffer) { delete lightingBuffer; lightingBuffer = nullptr; }
    if (iblFBO) glDeleteFramebuffers(1, &iblFBO);
    if (iblRBO) glDeleteRenderbuffers(1, &iblRBO);
    IBLManager::Destroy(ibl);
}

void PBRPipeline::HandleFBOs() {
    if (!framebuffer || !gBuffer) Init();
    if (framebuffer->GetWidth() != gBuffer->GetWidth() || framebuffer->GetHeight() != gBuffer->GetHeight())
        gBuffer->Resize(framebuffer->GetWidth(), framebuffer->GetHeight());
} 

void PBRPipeline::GeometryPass(Scene& scene) {
    gBuffer->Bind();
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    auto camera = GEngine->mainCamera;
    auto entities = scene.GetEntitiesWith<Transform, MeshRenderer>();

    Shader* s = &shaders.gbuffer;
    s->use(); 

    for (auto e : entities) {
        auto& tr = scene.GetComponent<Transform>(e);
        auto& mr = scene.GetComponent<MeshRenderer>(e);
        s->setMat4("model", tr.GetMatrix());

        for (auto& [material, meshes] : mr.batches) {
            material->Apply(s);               // deve setar albedo/metal/rough/ao e texturas
            for (auto& sub : meshes) {
                mr.DrawSubM(sub);
            }
        }
    }

    gBuffer->Unbind();
}

void PBRPipeline::BindGBufferTo(Shader* s) {
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, gBuffer->GetTextureByType(FramebufferTextureType::Position));
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, gBuffer->GetTextureByType(FramebufferTextureType::Normal));
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, gBuffer->GetTextureByType(FramebufferTextureType::Albedo));
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, gBuffer->GetTextureByType(FramebufferTextureType::MetallicRoughnessAO));
}

void PBRPipeline::BindIBLTo(Shader* s) {
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_CUBE_MAP, ibl.irradiance);
    glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_CUBE_MAP, ibl.prefilter);
    glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, ibl.brdfLUT);
}

void PBRPipeline::LightingPass(Scene& scene) {
    // acumulamos direto no framebuffer final
    framebuffer->Bind();
    glDisable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // 1) AMBIENTE (IBL) – full screen
    glDisable(GL_BLEND);
    shaders.iblAmbientShader.use();
    shaders.iblAmbientShader.setVec3("viewPos", GEngine->mainCamera->GetPosition());
    BindGBufferTo(&shaders.iblAmbientShader);
    BindIBLTo(&shaders.iblAmbientShader);
    RenderQuad();

    // BLEND aditivo (para pontos); direcional renderiza primeiro sem blend
    // 1) Luz direcional (fullscreen)
    //if (LightSystem::SetSunToShader(&shaders.dirLight) != 0) {
    //    shaders.dirLight.use();
    //    shaders.dirLight.setVec2("screenSize", glm::vec2(framebuffer->GetWidth(), framebuffer->GetHeight()));
    //    BindGBufferTo(&shaders.dirLight);
    //    BindIBLTo(&shaders.dirLight);
    //    // uniforms comuns (camPos para especular ao usar view-dependent BRDF)
    //    shaders.dirLight.setVec3("camPos", GEngine->mainCamera->GetPosition());
    //    RenderQuad();
    //}




    // 2) Luzes pontuais (light volumes instanciados) + IBL (o shader soma IBL toda passada) 

    // 2) LUZES PONTUAIS PBR (aditivo via esferas instanciadas)
    glEnable(GL_BLEND); glBlendEquation(GL_FUNC_ADD); glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_CULL_FACE); glCullFace(GL_BACK);

    Shader* pbrPointShader = &shaders.pointLight;
    pbrPointShader->use();
    pbrPointShader->setVec3("viewPos", GEngine->mainCamera->GetPosition());
    pbrPointShader->setVec2("screenSize", glm::vec2(GEngine->renderer->frameScreen->GetWidth(), GEngine->renderer->frameScreen->GetHeight()));
    pbrPointShader->setFloat("far_plane", 25); // adapte ao seu sistema

    BindGBufferTo(pbrPointShader);
    //BindIBLTo(pbrPointShader);

    // UBO de luzes já está bound no binding=1 (igual seu shader antigo)
    LightSystem::SetPointsToShader(pbrPointShader, 0); 
    // instâncias
    std::vector<LightInstanceData> instanceData;
    for (auto e : LightSystem::lightInActive) {
        auto& light = GEngine->scene->GetComponent<LightComponent>(e);
        auto& trans = GEngine->scene->GetComponent<Transform>(e);  
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






    /*glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_CULL_FACE); glCullFace(GL_BACK);*/
      
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);  
    //DrawSkybox();
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);         // não escreve no depth
}

void PBRPipeline::DrawSkybox() { 
    glDepthFunc(GL_LEQUAL);
    shaders.skybox.use();
    shaders.skybox.setMat4("view", GEngine->mainCamera->GetViewMatrix());
    shaders.skybox.setMat4("projection", GEngine->mainCamera->GetProjectionMatrix());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl.envCubemap); 
    RenderCube(); 
    glDepthFunc(GL_LESS);
}

void PBRPipeline::ForwardPass(Scene& scene) {
    // (opcional) transparências/partículas usando PBR forward com IBL:
    // copiar depth do gbuffer -> framebuffer se quiser ordenar contra opacos
    // glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer->GetID());
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer->GetID());
    // glBlitFramebuffer(0,0,w,h,0,0,w,h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    // ... desenhe transparentes com shader PBR forward e BindIBLTo()
}
