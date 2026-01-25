#include "Renderer.h" 
#include "../Core/EngineContext.h" 
#include "../ECS/Components/TransformComponent.h"
#include "../ECS/Components/MeshRenderer.h"

void Renderer::Init(SceneECS* scene) {  m_Scene = scene;   }

void Renderer::Initialize() { 
    try {  m_Scene->Load(); } catch (const std::exception& e) { std::cerr << "[EXCEÇÃO - Load] " << e.what() << "\n"; } 
}

void Renderer::BeginFrame() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
} 
 
void Renderer::RenderSceneWithShader(Shader* shaderOverride) {
    auto entities = GEngine->scene->GetEntitiesWith<TransformComponent, MeshRenderer>();

    for (Entity e : entities) {
        auto& transform = GEngine->scene->GetComponent<TransformComponent>(e);
        auto& meshRenderer = GEngine->scene->GetComponent<MeshRenderer>(e);

        shaderOverride->use();
        shaderOverride->setMat4("model", transform.GetMatrix());

        if(meshRenderer.mesh) meshRenderer.mesh->Draw();
    }
}

void Renderer::RenderSceneWithShader2(Shader* shaderOverride) {
    auto entities = GEngine->scene->GetEntitiesWith<TransformComponent, MeshRenderer>();

    for (Entity e : entities) {
        auto& transform = GEngine->scene->GetComponent<TransformComponent>(e);
        auto& meshRenderer = GEngine->scene->GetComponent<MeshRenderer>(e);

        shaderOverride->use();
        shaderOverride->setMat4("model", transform.GetMatrix());

        meshRenderer.mesh->Draw();
    }
}
  

// Fog
//Adicionado nos shaders(fragment) :
//    float fogFactor = exp(-fogDensity * fogDistance);
//    finalColor = mix(fogColor, finalColor, fogFactor);
