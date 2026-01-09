#pragma once
#include <vector>
#include <deque>
#include <unordered_map>
#include <iostream>

#include "../../../Graphics/Renderer.h" 

struct ShadowSettings {
    ShadowLOD resolution;
    std::deque<GLuint> availableDirectionalMaps;
    std::deque<GLuint> availablePointMaps;
};

class ShadowPool {
public:
    static void Init(); // cria FBOs, preenche pool vector
    static GLuint GetAvailableShadowMap(LightType type, ShadowLOD lod);
    static void ReturnShadowMapToPool(ShadowLOD lod, GLuint shadowMap, LightType type);
    static unsigned int GetSettingsForLOD(ShadowLOD lod);

    // mapa público (chave = entity -> pair<LOD, indexInArray>)
    static inline std::unordered_map<Entity, std::pair<ShadowLOD, unsigned int>> ShadowMaps;

    // FBO compartilhado para render de shadow maps individuais
    static inline GLuint depthMapFBO = 0;

private:
    static inline std::vector<ShadowSettings> poolShadowsTex;
    static void CreateShadowTexture2D(GLuint& outTex, int resolution);
    static void CreateShadowCube(GLuint& outTex, int resolution);
};
