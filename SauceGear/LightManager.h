#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include <glm/glm.hpp>
#include <vector>
#include "Shader.h" 

class LightManager {
public:
    LightManager(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& colors);
    void UpdateAndRender(const Shader& shader, float time);

private:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> colors;
};

#endif // LIGHT_MANAGER_H