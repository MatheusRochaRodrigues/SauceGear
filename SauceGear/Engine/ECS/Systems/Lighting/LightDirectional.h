#pragma once
#include "../../../ECS/Components/ComponentsHelper.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

class LightDirectional {
public:
    // Atualiza sombras de uma luz direcional comum (não cascades do sol)
    // depthFBO: FBO a ser usado (fornecido pelo pool)
    static void UpdateDirectional(LightComponent& light, const glm::vec3& pos, unsigned int resolution, GLuint depthFBO, GLuint texture);
};
