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
        {FramebufferTextureType::Albedo},              // 2 (rgb = baseColor)
        {FramebufferTextureType::MetallicRoughnessAO}, // 3 (r=metallic,g=roughness,b=ao)
        }, true);

    lightingBuffer = nullptr; // opcional

    sphereMesh = PrimitiveMesh::CreateSphere2RenderingLight();

    // FBO p/ IBL
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    // prepara IBL (com cache)
    ibl = IBLManager::EnsureIBL(currentHDR, cacheDir,
        shaders.equirect, shaders.irradiance, shaders.prefilter, shaders.brdf,
        captureFBO, captureRBO);

    // Shaders defaults
    shaders.skybox.use(); shaders.skybox.setInt("environmentMap", 0);

    // Lighting shaders usam samplers fixos:
    shaders.dirLight.use();
    shaders.dirLight.setInt("gPosition", 0);
    shaders.dirLight.setInt("gNormal", 1);
    shaders.dirLight.setInt("gAlbedo", 2);
    shaders.dirLight.setInt("gMRAO", 3);
    shaders.dirLight.setInt("irradianceMap", 4);
    shaders.dirLight.setInt("prefilterMap", 5);
    shaders.dirLight.setInt("brdfLUT", 6);

    shaders.pointLight.use();
    shaders.pointLight.setInt("gPosition", 0);
    shaders.pointLight.setInt("gNormal", 1);
    shaders.pointLight.setInt("gAlbedo", 2);
    shaders.pointLight.setInt("gMRAO", 3);
    shaders.pointLight.setInt("irradianceMap", 4);
    shaders.pointLight.setInt("prefilterMap", 5);
    shaders.pointLight.setInt("brdfLUT", 6);
}

void PBRPipeline::Shutdown() {
    if (sphereMesh) sphereMesh = nullptr;
    if (gBuffer) { delete gBuffer; gBuffer = nullptr; }
    if (framebuffer) { delete framebuffer; framebuffer = nullptr; }
    if (lightingBuffer) { delete lightingBuffer; lightingBuffer = nullptr; }
    if (captureFBO) glDeleteFramebuffers(1, &captureFBO);
    if (captureRBO) glDeleteRenderbuffers(1, &captureRBO);
    IBLManager::Destroy(ibl);
}

void PBRPipeline::HandleFBOs() {
    if (!framebuffer || !gBuffer) Init();
    if (framebuffer->GetWidth() != gBuffer->GetWidth() || framebuffer->GetHeight() != gBuffer->GetHeight())
        gBuffer->Resize(framebuffer->GetWidth(), framebuffer->GetHeight());
}

void PBRPipeline::Render(Scene& scene) {
    HandleFBOs();
    GeometryPass(scene);
    LightingPass(scene);
    ForwardPass(scene);

    // saída final
    GEngine->renderer->GetTextureRendered = framebuffer->GetTexture(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    Shader* s = &shaders.gbufferVSFS;
    s->use();
    s->setMat4("view", camera->GetViewMatrix());
    s->setMat4("projection", camera->GetProjectionMatrix());

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

    // BLEND aditivo (para pontos); direcional renderiza primeiro sem blend
    // 1) Luz direcional (fullscreen)
    if (LightSystem::SetSunToShader(&shaders.dirLight) != 0) {
        shaders.dirLight.use();
        shaders.dirLight.setVec2("screenSize", glm::vec2(framebuffer->GetWidth(), framebuffer->GetHeight()));
        BindGBufferTo(&shaders.dirLight);
        BindIBLTo(&shaders.dirLight);
        // uniforms comuns (camPos para especular ao usar view-dependent BRDF)
        shaders.dirLight.setVec3("camPos", GEngine->mainCamera->GetPosition());
        RenderQuad();
    }

    // 2) Luzes pontuais (light volumes instanciados) + IBL (o shader soma IBL toda passada)
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    shaders.pointLight.use();
    shaders.pointLight.setVec2("screenSize", glm::vec2(framebuffer->GetWidth(), framebuffer->GetHeight()));
    BindGBufferTo(&shaders.pointLight);
    BindIBLTo(&shaders.pointLight);
    shaders.pointLight.setVec3("camPos", GEngine->mainCamera->GetPosition());

    // UBO/SSBO de luzes já setado via LightSystem::SetPointsToShader()
    LightSystem::SetPointsToShader(&shaders.pointLight);

    std::vector<LightInstanceData> instances;
    instances.reserve(LightSystem::lightInActive.size());
    for (auto ent : LightSystem::lightInActive) {
        auto& L = scene.GetComponent<LightComponent>(ent);
        auto& Tr = scene.GetComponent<Transform>(ent);
        // seu cálculo de raio:
        const float rangeLight = 0.01f;
        float maxB = std::fmax(std::fmax(L.color.r, L.color.g), L.color.b);
        float radius = std::sqrt(maxB / rangeLight);
        instances.push_back({ Tr.position, radius, L.depthMap });
    }

    sphereMesh->SetInstanceData(
        instances.data(),
        instances.size() * sizeof(LightInstanceData),
        { {10,3}, {11,1}, {12,1} }
    );
    sphereMesh->DrawInstanced(instances.size());

    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    // 3) Skybox (no final pra não interferir na iluminação)
    DrawSkybox();

    glEnable(GL_DEPTH_TEST);
}

void PBRPipeline::DrawSkybox() {
    glDepthFunc(GL_LEQUAL);
    shaders.skybox.use();
    shaders.skybox.setMat4("view", GEngine->mainCamera->GetViewMatrix());
    shaders.skybox.setMat4("projection", GEngine->mainCamera->GetProjectionMatrix());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl.envCubemap);
    renderCube(); // seu util
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
