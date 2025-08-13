#include "SkyboxRenderer.h"
//_____First Way
//---------------------------------------------------------------
SkyboxRenderer::SkyboxRenderer(const Shader& shader, const glm::mat4& projection)
    : shader(shader), projection(projection){
    this->shader.use();
    this->shader.setInt("environmentMap", 0);
    this->shader.setMat4("projection", projection);
}

void SkyboxRenderer::Draw(const glm::mat4& view, GLuint cubemap) {
    //glDepthFunc(GL_LEQUAL);

    shader.use();
    shader.setMat4("view", glm::mat4(glm::mat3(view))); // remove translation 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap); 

    renderCube();

    //glDepthFunc(GL_LESS);
}


//_____Second Way
//---------------------------------------------------------------
SkyboxRenderer::SkyboxRenderer(const Shader& shader)
    : shader(shader) {
    this->shader.use();
    this->shader.setInt("environmentMap", 0);
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