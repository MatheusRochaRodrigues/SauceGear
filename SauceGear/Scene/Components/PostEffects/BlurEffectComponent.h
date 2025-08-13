#pragma once
#include "../PostProcessComponent.h"
#include "../../../Graphics/FullscreenQuad.h"

struct BlurEffectComponent : public PostProcessComponent {
    glm::vec2 direction;

    BlurEffectComponent(Shader* shader, glm::vec2 dir) : direction(dir) {
        this->shader = shader;
    }

    void Apply() override { 
        shader->setInt("scene", 0);
        shader->setVec2("u_Direction", direction); 
};






//void Apply(GLuint* currentTex) override {
//    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
//    glClear(GL_COLOR_BUFFER_BIT);
//
//    shader->use();
//    shader->setInt("scene", 0);
//    shader->setVec2("u_Direction", direction);
//
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, inputTexture);
//    Render();               //FullscreenQuad
//
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}