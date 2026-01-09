#include "LightPoint.h"
#include "../../../Core/EngineContext.h"
#include "../../../Graphics/Renderer.h" 

void LightPoint::UpdatePoint(LightComponent& light, const glm::vec3& pos, unsigned int resolution, unsigned int depthFBO, unsigned int texture) {
    Shader* pointShadow = GEngine->renderer->GetShadowShader_Point;
    float nearPlane = 0.1f, farPlane = light.range;

    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);
    glm::vec3 dirs[6] = {
        {1,0,0}, {-1,0,0}, {0,1,0},
        {0,-1,0}, {0,0,1}, {0,0,-1}
    };
    glm::vec3 ups[6] = {
        {0,-1,0}, {0,-1,0}, {0,0,1},
        {0,0,-1}, {0,-1,0}, {0,-1,0}
    };

    glm::mat4 shadowTransforms[6];
    for (int i = 0; i < 6; ++i)
        shadowTransforms[i] = shadowProj * glm::lookAt(pos, pos + dirs[i], ups[i]);

    pointShadow->use();
    for (int i = 0; i < 6; ++i)
        pointShadow->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
    pointShadow->setVec3("lightPos", pos);
    pointShadow->setFloat("far_plane", farPlane);

    glViewport(0, 0, resolution / 2, resolution / 2);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "[LightPoint] framebuffer incomplete: " << status << std::endl;
    }
    glClear(GL_DEPTH_BUFFER_BIT);

    GEngine->renderer->RenderSceneWithShader(pointShadow);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GEngine->window->SetWindowViewport0();
}
