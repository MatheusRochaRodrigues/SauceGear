#include"LightUtils.h" 
#include <algorithm>
#include "../../ECS/Components/LightComponent.h" 
#include "../../ECS/Components/TransformComponent.h" 
#include "../../Scene/SceneECS.h"
#include "../../Core/EngineContext.h"
#include "ShadowPool.h"

namespace HandleLights {
    std::vector<Entity> SelectLightsToCastShadow(const std::vector<Entity>& lights) {
        std::vector<Entity> selectedEntities;
        int Directional = 0;
        int Point = 0;
        for (const auto& light : lights) {
            const auto& lc = GEngine->scene->GetComponent<LightComponent>(light);
            if (!lc.castShadow) continue;
            switch (lc.type) {
            case LightType::Directional:
                if (Directional >= MAX_LIGHTS) break;
                selectedEntities.push_back(light);
                Directional++;
                break;
            case LightType::Point:
                if (Point >= MAX_LIGHTS) break;
                selectedEntities.push_back(light);
                Point++;
                break;
            default:
                break;
            }
        }
        return selectedEntities;
    }

    LightGroups GroupLightsByType(const std::vector<Entity>& lights) {
        LightGroups groups;
        for (Entity e : lights) {
            const auto& light = GEngine->scene->GetComponent<LightComponent>(e);
            switch (light.type) {
            case LightType::Directional: groups.directional.push_back(e); break;
            case LightType::Point:       groups.point.push_back(e);       break;
            case LightType::Spot:        groups.spot.push_back(e);        break;
            }
        }
        return groups;
    }

    std::vector<Entity> GetClosestLights(const glm::vec3& playerPos) {
        auto all = GEngine->scene->GetEntitiesWith<LightComponent, TransformComponent>();

        std::vector<std::pair<float, Entity>> sorted;
        sorted.reserve(all.size());

        for (Entity e : all) {
            const auto& transform = GEngine->scene->GetComponent<TransformComponent>(e);
            float dist = glm::distance(playerPos, transform.position);
            sorted.emplace_back(dist, e);
        }

        std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) {
            return a.first < b.first;
            });

        std::vector<Entity> result;
        for (int i = 0; i < std::min(MAX_LIGHTS, (int)sorted.size()); ++i) {
            result.push_back(sorted[i].second);
        }

        return result;
    }

    //LOD Handlers
    ShadowLOD ComputeLOD(float distance) {
        if (distance < 25.0f) return ShadowLOD::HIGH;
        if (distance < 40.0f) return ShadowLOD::MEDIUM;
        if (distance < 60.0f) return ShadowLOD::LOW;
        return ShadowLOD::NONE;
    }

    unsigned int GetSettingsForLOD(ShadowLOD lod) {
        return ShadowPool::GetSettingsForLOD(lod);
    }
}