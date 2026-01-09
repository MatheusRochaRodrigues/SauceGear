#pragma once
//#include "../Scene/SceneECS.h" 
#include "../Scene/Entity.h" 
#include "../Scene/Components/ComponentsHelper.h" 
#include "../Graphics/Shader.h" 
#include "../../Core/EngineContext.h" 

inline void SetLightsToShader(Shader& shader, const std::vector<Entity>& lights) {
    int lightCount = 0;

    for (Entity e : lights) {
        const auto& light = GEngine->scene->GetComponent<LightComponent>(e);
        const auto& transform = GEngine->scene->GetComponent<Transform>(e);

        std::string prefix = "lights[" + std::to_string(lightCount) + "]";

        shader.setVec3 (prefix + ".position", transform.position);
        shader.setVec3 (prefix + ".color", light.color);
        shader.setInt  (prefix + ".type", static_cast<int>(light.type));
        shader.setFloat(prefix + ".intensity", light.intensity);

        if (light.type == LightComponent::Type::Spot) {
            shader.setFloat(prefix + ".cutoff", light.cutoff);
            //shader.setFloat(prefix + ".outerCutoff", light.outerCutoff);
        }

        if (light.type != LightComponent::Type::Directional)
            shader.setFloat(prefix + ".range", light.range);

        ++lightCount;
        if (lightCount >= MAX_LIGHTS) break;
    }

    shader.setInt("numLights", lightCount);
}