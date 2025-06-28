#ifndef SKYBOX_RENDERER_H
#define SKYBOX_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "RenderUtils.h" // renderCube()

class SkyboxRenderer {
public:
    SkyboxRenderer(const Shader& shader);
    void Draw(const glm::mat4& view, const glm::mat4& projection, GLuint cubemap);

private:
    Shader shader;
};

#endif