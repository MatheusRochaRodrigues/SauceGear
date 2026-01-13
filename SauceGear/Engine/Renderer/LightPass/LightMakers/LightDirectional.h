#pragma once
#include <glm/glm.hpp>
#include "../../../ECS/Components/LightComponent.h" 

class LightDirectional {
public:
    // Atualiza sombras de uma luz direcional comum (não cascades do sol)
    // depthFBO: FBO a ser usado (fornecido pelo pool)
    static void UpdateDirectional(LightComponent& light, const glm::vec3& pos, unsigned int resolution, GLuint depthFBO, GLuint texture);
};
