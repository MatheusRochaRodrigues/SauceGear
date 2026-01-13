#pragma once
#include <glm/glm.hpp> 
#include "../../../ECS/Components/LightComponent.h"

class LightPoint {
public:
    // depthFBO: FBO do pool para bindar.
    static void UpdatePoint(LightComponent& light, const glm::vec3& pos, unsigned int resolution, unsigned int depthFBO, unsigned int texture);
};
