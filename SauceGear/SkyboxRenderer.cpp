#include "SkyboxRenderer.h"

SkyboxRenderer::SkyboxRenderer(const Shader& shader)
    : shader(shader) {
}

void SkyboxRenderer::Draw(const glm::mat4& view, const glm::mat4& projection, GLuint cubemap) {
    //glDepthFunc(GL_LEQUAL);

    shader.use();
    shader.setMat4("view", glm::mat4(glm::mat3(view))); // remove translation
    shader.setMat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    shader.setInt("environmentMap", 0);

    renderCube();

    //glDepthFunc(GL_LESS);
}