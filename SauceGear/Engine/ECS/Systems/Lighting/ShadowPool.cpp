#include "ShadowPool.h" 
#include "../../../Core/EngineContext.h"
#include "../../../ECS/Components/ComponentsHelper.h"

void ShadowPool::Init() {
    if (depthMapFBO != 0) return; // já inicializado

    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    poolShadowsTex.clear();
    poolShadowsTex.push_back(ShadowSettings{ ShadowLOD::HIGH, std::deque<GLuint>(), std::deque<GLuint>() });
    poolShadowsTex.push_back(ShadowSettings{ ShadowLOD::MEDIUM, std::deque<GLuint>(), std::deque<GLuint>() });
    poolShadowsTex.push_back(ShadowSettings{ ShadowLOD::LOW, std::deque<GLuint>(), std::deque<GLuint>() });

    std::cout << "[ShadowPool] initialized\n";
}

unsigned int ShadowPool::GetSettingsForLOD(ShadowLOD lod) {
    switch (lod) {
    case ShadowLOD::HIGH: return 1024;
    case ShadowLOD::MEDIUM: return 512;
    case ShadowLOD::LOW: return 256;
    default: return 0;
    }
}

void ShadowPool::CreateShadowTexture2D(GLuint& outTex, int resolution) {
    glGenTextures(1, &outTex);
    glBindTexture(GL_TEXTURE_2D, outTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f,1.0f,1.0f,1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
}

void ShadowPool::CreateShadowCube(GLuint& outTex, int resolution) {
    glGenTextures(1, &outTex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, outTex);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

GLuint ShadowPool::GetAvailableShadowMap(LightType type, ShadowLOD lod) {
    ShadowSettings& settings = poolShadowsTex[static_cast<int>(lod)];
    GLuint shadowMap = 0;
    int resolution = GetSettingsForLOD(lod);

    if (type == LightType::Directional) {
        if (!settings.availableDirectionalMaps.empty()) {
            shadowMap = settings.availableDirectionalMaps.front();
            settings.availableDirectionalMaps.pop_front();
        }
        else {
            CreateShadowTexture2D(shadowMap, resolution);
        }
    }
    else if (type == LightType::Point) {
        if (!settings.availablePointMaps.empty()) {
            shadowMap = settings.availablePointMaps.front();
            settings.availablePointMaps.pop_front();
        }
        else {
            // para point lights usamos metade da resolução (como antes)
            CreateShadowCube(shadowMap, std::max(1, resolution / 2));
        }
    }

    return shadowMap;
}

void ShadowPool::ReturnShadowMapToPool(ShadowLOD lod, GLuint shadowMap, LightType type) {
    ShadowSettings& settings = poolShadowsTex[static_cast<int>(lod)];
    if (type == LightType::Directional) {
        settings.availableDirectionalMaps.push_back(shadowMap);
    }
    else if (type == LightType::Point) {
        settings.availablePointMaps.push_back(shadowMap);
    }
}
