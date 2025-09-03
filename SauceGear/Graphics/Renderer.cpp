#include "Renderer.h" 
#include "../Core/EngineContext.h" 

void Renderer::Init(SceneECS* scene) {
    m_Scene = scene;  
}

void Renderer::Initialize() { 
    try {  m_Scene->Load(); } catch (const std::exception& e) { std::cerr << "[EXCEÇÃO - Load] " << e.what() << "\n"; }

    //pbrShader = Shader("PBR/pbr.vs", "PBR/pbr.fs");
    //ibl = new IBLMapGenerator(/* shaders */);
    //ibl->Generate("resources/Textures/hdr/tst/rogland_moonlit_night_4k.hdr");
    //skybox = new SkyboxRenderer(/* shader, projection */);

    //baseColorShader = Shader("BlinnPhong/BaseShadow.vs", "BlinnPhong/BaseShadow.fs");
}

void Renderer::BeginFrame() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
 

//DrawWithShader() - Usa um shader override (útil para render shadow map, GBuffer, etc). 
void Renderer::RenderSceneWithShader(Shader* shaderOverride) {
    auto entities = GEngine->scene->GetEntitiesWith<Transform, MeshRenderer>();

    for (Entity e : entities) {
        auto& transform = GEngine->scene->GetComponent<Transform>(e);
        auto& meshRenderer = GEngine->scene->GetComponent<MeshRenderer>(e);

        shaderOverride->use();
        shaderOverride->setMat4("model", transform.GetMatrix());

        meshRenderer.mesh->Draw(); 
    }
}
 

//Use glDepthFunc(GL_LEQUAL) para desenhar atrás de tudo
void Renderer::RenderSkybox() { //const Camera& camera
    // shader->setMat4("view", glm::mat4(glm::mat3(camera.GetViewMatrix())));
    // shader->setMat4("projection", camera.GetProjectionMatrix());
    // bind cubemap + draw cube
}


// Fog
//Adicionado nos shaders(fragment) :
//    float fogFactor = exp(-fogDensity * fogDistance);
//    finalColor = mix(fogColor, finalColor, fogFactor);
