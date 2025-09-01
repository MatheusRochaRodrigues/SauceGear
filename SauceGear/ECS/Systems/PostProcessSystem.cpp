#include "PostProcessSystem.h" 
#include "../../Graphics/Framebuffer.h"
 
PostProcessSystem::PostProcessSystem() {
    shaderView = new Shader("PostProcess/post.vs", "PostProcess/post.fs");
    //passes.push_back(new BlurEffectComponent(new Shader("post.vert", "blur.frag"), glm::vec2(1, 0)));
}

void PostProcessSystem::initialize(PostProcessComponent& pp) {    //PostProcessSystem
    unsigned int width = GEngine->window->GetWidth();
    unsigned int height = GEngine->window->GetHeight();
    pp.outputFBO = new Framebuffer(width, height, { { FramebufferTextureType::ColorRGB } }, true);
    //PostProcessComponent::outputFBO = new Framebuffer(width, height, { {FramebufferTextureType::Float16} });
}

void PostProcessSystem::Resize(int w, int h) {
    /*width = w;
    height = h;
    for (auto& pass : passes)
        pass.Resize(w, h);*/
}

void PostProcessSystem::AddPass(PostProcessComponent* pass) {
    passes.emplace_back(pass);                              //passes.emplace_back(width, height, fragPath);
} 

void PostProcessSystem::InitFullscreenQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void PostProcessSystem::DrawQuad() {
    if (quadVAO == 0) InitFullscreenQuad();
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

 
GLuint PostProcessSystem::GetFinalTexture() const {
    return passes.empty() ? 0 : passes.back()->GetOutputTexture();
}








//void PostProcessSystem::Update() {
//    SceneECS& scene = *GEngine->scene;
//    auto entities = scene.GetEntitiesWith<PostProcessComponent>();
//    for (auto entity : entities) {
//        auto& pp = scene.GetComponent<PostProcessComponent>(entity);
//        pp.Apply();  // chama a lógica individual de cada efeito
//
//        glBindFramebuffer(GL_FRAMEBUFFER, pp.outputFBO);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        pp.shader->use();
//        pp.shader->setInt("scene", 0);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, pp.inputTexture);
//
//        DrawQuad();
//
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//    }
//}