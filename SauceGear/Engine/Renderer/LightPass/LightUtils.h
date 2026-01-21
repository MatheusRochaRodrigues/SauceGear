#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../../Scene/Entity.h"
#include "../../ECS/Components/LightComponent.h"  

struct LightGroups {
    std::vector<Entity> directional;
    std::vector<Entity> point;
    std::vector<Entity> spot;

    //std::unordered_map<Entity, GLuint> indexPoint;
};

namespace HandleLights{

    template <typename Func>
    void ForEachLight(LightGroups& lights, Func func) {
        for (auto& e : lights.directional) func(e);
        for (auto& e : lights.point)       func(e);
        for (auto& e : lights.spot)        func(e);
    }

    ShadowLOD ComputeLOD(float distance);
    unsigned int GetSettingsForLOD(ShadowLOD lod);

    // seleção/agrupamento util
    std::vector<Entity> SelectLightsToCastShadow(const std::vector<Entity>& lights);
    LightGroups         GroupLightsByType(const std::vector<Entity>& lights);
    std::vector<Entity> GetClosestLights(const glm::vec3& playerPos);
}