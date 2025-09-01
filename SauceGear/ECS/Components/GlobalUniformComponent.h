#pragma once

#include <glm/glm.hpp> 

struct GlobalUniforms {
    glm::mat4 view;
    glm::mat4 projection;
    //float time;
    glm::vec3 cameraPosition;
    // Padding para alinhamento se necessário!
};
