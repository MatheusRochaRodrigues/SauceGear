#pragma once
#include <vector>  
#include <glm/glm.hpp>  
#include "../../ECS/Components/LightComponent.h"  
#include "../../ECS/Components/TransformComponent.h"  
#include "../../Graphics/Shader.h"  
#include "LightUtils.h"

#define MAX_LIGHTS_PROJECTION 3
#define MAX_CASCADES 4
#define MAX_LIGHTS 16 

class LightPass {
public:
    LightPass();
    ~LightPass() = default;
     
    void Update(bool updtSun = true);

    void SetLightsToSSBO();

    // Lógica para devolver sombras que não estão mais ativas
    void HandleShadowMapReturn(const glm::vec3& playerPosition); 

    // estados públicos (semântica preservada)
    static inline Entity                    currentSun = INVALID_ENTITY;
    static inline std::vector<glm::mat4>    lightSpaceMatrices;
    static inline std::vector<float>        shadowCascadeLevels;

    // light-in-active (lista de luzes que renderizamos no frame)
    static inline LightGroups lightInActive;  

    static void OnEntityDestroyed(Entity e);
private:
    GLuint depthMapFBO = 0; // framebuffer usado para renderizar sombras 2D / cube (delegado para pool)
    GLuint lightSSBO = 0; 

    // friend modules to allow uso interno do depthMapFBO se necessário
    friend class LightShadowPool;
    friend class LightDirectional;
    friend class LightPoint;
    friend class CascadeSun;
};



/*
caso singleton se eu preferir

LightPass& LightPass::Get()
{
    static LightPass instance;
    return instance;
}


*/