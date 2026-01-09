#pragma once
#include <vector>
#include <unordered_map>
#include <deque>
#include <iostream>

#include "../../Core/EngineContext.h"
#include "../../Scene/SceneECS.h"
#include "../../ECS/Components/ComponentsHelper.h" // garante LightComponent, Transform, etc.
#include "ShadowPool.h"
#include "LightDirectional.h"
#include "LightPoint.h"
#include "CascadeSun.h"

#define MAX_LIGHTS_PROJECTION 3
#define MAX_CASCADES 4
#define MAX_LIGHTS 16

using Scene = SceneECS;

struct LightGroups {
    std::vector<Entity> directional;
    std::vector<Entity> point;
    std::vector<Entity> spot;
};

class LightSystem : public System {
public:
    LightSystem();
    ~LightSystem() = default;

    void Init(); // inicializações necessárias
    void Update(float dt) override;

    // utilidades para enviar dados aos shaders (mantive a interface)
    static int SetSunToShader(Shader* shader);
    static void SetPointsToShader(Shader* shader, int i = 0);
    static void SetLightsToShader(Shader* shader);

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
    LightGroups GroupLightsByType(const std::vector<Entity>& lights);
    std::vector<Entity> GetClosestLights(const glm::vec3& playerPos);

    // estados públicos (semântica preservada)
    static inline Entity currentSun = INVALID_ENTITY;
    static inline std::vector<glm::mat4> lightSpaceMatrices;
    static inline std::vector<float> shadowCascadeLevels;

    // light-in-active (lista de luzes que renderizamos no frame)
    static inline LightGroups lightInActive;

private:
    GLuint depthMapFBO = 0; // framebuffer usado para renderizar sombras 2D / cube (delegado para pool)
    GLuint lightSSBO = 0;
    unsigned int offBuff = 144;

    void verify();

    // helpers
    ShadowLOD ComputeLOD(float distance);
    unsigned int GetSettingsForLOD(ShadowLOD lod);

    // friend modules to allow uso interno do depthMapFBO se necessário
    /*friend class LightShadowPool;
    friend class LightDirectional;
    friend class LightPoint;*/
    //friend class CascadeSun;
};
