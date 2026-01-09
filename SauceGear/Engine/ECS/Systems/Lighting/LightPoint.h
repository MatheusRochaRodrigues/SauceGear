#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "../../../ECS/Components/ComponentsHelper.h"

class LightPoint {
public:
    // depthFBO: FBO do pool para bindar.
    static void UpdatePoint(LightComponent& light, const glm::vec3& pos, unsigned int resolution, unsigned int depthFBO, unsigned int texture);
};
