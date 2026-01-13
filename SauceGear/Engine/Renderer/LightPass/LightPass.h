#pragma once
#include <vector>
#include <unordered_map>
#include <deque>
#include <iostream>

#include "../../Core/EngineContext.h"
#include "../../Scene/SceneECS.h"
#include "../../ECS/Components/ComponentsHelper.h" // garante LightComponent, Transform, etc.

#include "ShadowPool.h"
#include "LightMakers/LightDirectional.h"
#include "LightMakers/LightPoint.h"
#include "LightMakers/CascadeSun.h"

#define MAX_LIGHTS_PROJECTION 3
#define MAX_CASCADES 4
#define MAX_LIGHTS 16

using Scene = SceneECS;

struct LightGroups {
    std::vector<Entity> directional;
    std::vector<Entity> point;
    std::vector<Entity> spot;
};

class LightPass {
public:
    LightPass();
    ~LightPass() = default;
     
    void Update();

    template <typename Func>
    static void ForEachLight(LightGroups& lights, Func func) {
        for (auto& e : lights.directional) func(e);
        for (auto& e : lights.point)       func(e);
        for (auto& e : lights.spot)        func(e);
    }

    void SetLightsToSSBO();

    // Lógica para devolver sombras que não estão mais ativas
    void HandleShadowMapReturn(const glm::vec3& playerPosition);

    // seleção/agrupamento util
    std::vector<Entity> SelectLightsToCastShadow(const std::vector<Entity>& lights);
    LightGroups         GroupLightsByType(const std::vector<Entity>& lights);
    std::vector<Entity> GetClosestLights(const glm::vec3& playerPos);

    // estados públicos (semântica preservada)
    static inline Entity                    currentSun = INVALID_ENTITY;
    static inline std::vector<glm::mat4>    lightSpaceMatrices;
    static inline std::vector<float>        shadowCascadeLevels;

    // light-in-active (lista de luzes que renderizamos no frame)
    static inline LightGroups lightInActive; 

    //------------------------------------ utilidades para enviar dados aos shaders (mantive a interface) 
    static int SetSunToShader(Shader* shader) {
        if (currentSun == INVALID_ENTITY) return 0;
        shader->use();
        auto& light = GEngine->scene->GetComponent<LightComponent>(currentSun);
        auto& transform = GEngine->scene->GetComponent<TransformComponent>(currentSun);

        shader->setFloat("farPlane", GEngine->mainCamera->farClip);

        std::string prefix = "light";
        shader->setVec3(prefix + ".color", light.color);
        shader->setVec3(prefix + ".direction", transform.GetForwardDirection());
        shader->setMat4("view", GEngine->mainCamera->GetViewMatrix()); // view da câmera

        // usa CascadeSun diretamente
        const auto& cascadeDistances = CascadeSun::GetCascadeDistances();
        shader->setFloatArray("cascadePlaneDistances", cascadeDistances.data(), static_cast<int>(cascadeDistances.size())); // Envia o array inteiro de uma vez
        shader->setInt("cascadeCount", (int)cascadeDistances.size());

        // vincula a texture array do shadow map
        shader->setInt("cascadeShadowMap", 7);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D_ARRAY, CascadeSun::cascadeDepthMapArray);

        return 1;
    } 

    static void SetPointsToShader(Shader* shader, int i = 0) {
        shader->use();
        for (auto& [lightEntity, map] : ShadowPool::ShadowMaps) {
            int lightCount = map.second + i;
            auto& light = GEngine->scene->GetComponent<LightComponent>(lightEntity);
            if (light.type == LightType::Point) {
                shader->setInt("pointShadows[" + std::to_string(lightCount) + "]", lightCount);
                glActiveTexture(GL_TEXTURE0 + lightCount);
                glBindTexture(GL_TEXTURE_CUBE_MAP, light.depthMap);
            }
        }
        shader->setFloat("far_plane", 25.0f);
        shader->setInt("numLights", (int)ShadowPool::ShadowMaps.size());
        shader->setVec3("viewPos", GEngine->mainCamera->Position);
    }

    void SetLightsToShader(Shader* shader) {
        shader->use();
        for (auto& [lightEntity, map] : ShadowPool::ShadowMaps) {
            int lightCount = map.second;
            auto& light = GEngine->scene->GetComponent<LightComponent>(lightEntity);
            if (light.type == LightType::Point) {
                shader->setInt("pointShadows[" + std::to_string(lightCount) + "]", lightCount);
                glActiveTexture(GL_TEXTURE0 + lightCount);
                glBindTexture(GL_TEXTURE_CUBE_MAP, light.depthMap);
            }
            else {
                shader->setInt("shadowMaps[" + std::to_string(lightCount) + "]", lightCount);
                glActiveTexture(GL_TEXTURE0 + lightCount);
                glBindTexture(GL_TEXTURE_2D, light.depthMap);
            }
        }

        shader->setFloat("far_plane", 25.0f);
        shader->setInt("numLights", (int)ShadowPool::ShadowMaps.size());
        shader->setInt("albedoMap", 0);
        shader->setVec3("viewPos", GEngine->mainCamera->Position);
    }

    static void set_uShadowData(Shader& lightingShader, int unit = 10) {
        //shadows
        lightingShader.setInt("cascadeShadowMap", unit); // deve bater com GL_TEXTURE4
        // --- Shadow map array (sampler2DArray)
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D_ARRAY, CascadeSun::cascadeDepthMapArray);

        // --- Cascade count
        lightingShader.setInt("cascadeCount", CascadeSun::GetCascadeCount());

        // --- Cascade distances
        auto& distances = CascadeSun::GetCascadeDistances();
        lightingShader.setFloatArray("cascadePlaneDistances", distances.data(), (int)distances.size());

        // --- View matrix (para converter world→view)
        lightingShader.setMat4("view", GEngine->mainCamera->GetViewMatrix());
    }


private:
    GLuint depthMapFBO = 0; // framebuffer usado para renderizar sombras 2D / cube (delegado para pool)
    GLuint lightSSBO = 0;
    unsigned int offBuff = 144;

    void verify();

    // helpers
    ShadowLOD ComputeLOD(float distance);
    unsigned int GetSettingsForLOD(ShadowLOD lod);

    // friend modules to allow uso interno do depthMapFBO se necessário
    friend class LightShadowPool;
    friend class LightDirectional;
    friend class LightPoint;
    friend class CascadeSun;
};
