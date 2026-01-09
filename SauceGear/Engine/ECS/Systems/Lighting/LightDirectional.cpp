#include "LightDirectional.h"
#include "ShadowPool.h"
#include "../../../Core/EngineContext.h"
#include "../../../Graphics/Renderer.h" 

void LightDirectional::UpdateDirectional(LightComponent& light, const glm::vec3& pos, unsigned int resolution, GLuint depthFBO, GLuint texture) {
    Shader* shadowShader = GEngine->renderer->GetShadowShader_Directional;
    glm::mat4 lightProjection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 1.f, 7.5f);
    glm::mat4 lightView = glm::lookAt(pos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    light.lightSpaceMatrix = lightProjection * lightView;
    light.position = pos;

    shadowShader->use();
    shadowShader->setMat4("lightSpaceMatrix", light.lightSpaceMatrix);

    glViewport(0, 0, resolution, resolution);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "[LightDirectional] framebuffer incomplete: " << status << std::endl;
    }
    glClear(GL_DEPTH_BUFFER_BIT);

    GEngine->renderer->RenderSceneWithShader(shadowShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GEngine->window->SetWindowViewport0();
}
