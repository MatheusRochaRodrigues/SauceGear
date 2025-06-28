#include "LightManager.h"

LightManager::LightManager(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& colors)
    : positions(positions), colors(colors) {
}

void LightManager::UpdateAndRender(const Shader& shader, float time) {
    //for (size_t i = 0; i < positions.size(); ++i) {
    //    glm::vec3 pos = positions[i];
    //    // pos.x += sin(time * 5.0f) * 5.0f; // opcional, para movimento

    //    shader.setVec3("lightPositions[" + std::to_string(i) + "]", pos);
    //    shader.setVec3("lightColors[" + std::to_string(i) + "]", colors[i]);

    //    glm::mat4 model = glm::mat4(1.0f);
    //    model = glm::translate(model, pos);
    //    model = glm::scale(model, glm::vec3(0.5f));
    //    shader.setMat4("model", model);
    //    shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
    //    renderSphere();
    //}
}
