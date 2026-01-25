#pragma once
#include "../../ECS/Components/TransformComponent.h"
#include "../../ECS/Components/LightComponent.h"
#include "../../ECS/Components/ComputeSyncComponent.h"
#include "../../Scene/SceneECS.h"
#include "../../ECS/System.h"
#include "../../Renderer/RendererPBR/IBL/IBLManager.h" 
#include "../../Graphics/ComputeShader.h"
#include "../../Core/EngineContext.h" 

#include "../../Renderer/LightPass/LightPass.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/common.hpp>
#include <string>
#include <functional>
#include <algorithm>  // necessário para std::max

using std::string;

// smoothstep helper
static float my_smoothstep(float edge0, float edge1, float x) {
    float t = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

struct DayNightCycle {
    float timeOfDay = 12.0f;
    float daySpeed = 0.75f;     //0.05f;

    float exposureNight = 0.25f;
    float exposureDawn = 0.6f;
    float exposureDay = 1.3f;
    float exposureDusk = 0.7f;

    glm::vec3 nightColor = { 0.05f, 0.05f, 0.1f };
    glm::vec3 dawnColor = { 0.9f, 0.45f, 0.3f };
    glm::vec3 dayColor = { 1.0f, 1.0f, 0.95f };
    glm::vec3 duskColor = { 0.9f, 0.4f, 0.2f };

    std::pair<string, IBLSet> nightHDR;
    std::pair<string, IBLSet> dawnHDR;
    std::pair<string, IBLSet> dayHDR;
    std::pair<string, IBLSet> duskHDR;

    IBLSet* prevIBL = nullptr;
    IBLSet* nextIBL = nullptr;
    float currentExposure = 1.0f;
    glm::vec3 currentSkyTint = { 1.0f,1.0f,1.0f };
    float t = 0.0f;
};

struct DayNightSystem : public System {

    struct CubemapJob {
        GLuint prevTex;
        GLuint nextTex;
        GLuint outTex;
        int size;
        int mipCount;
        bool active = false;
    };

    DayNightCycle s_cycle;

    void Init() { 
        // pré-carrega os HDRs em IBL sets
        s_cycle.nightHDR = { "Assets/HDR/uploads_files_3782135_Anime+sky.hdr", {} };
        s_cycle.dawnHDR =  { "Assets/HDR/uploads_files_3782135_Anime+sky.hdr", {} };
        s_cycle.dayHDR = { "Engine/Resources/Textures/hdr/tst/Kloppenheim (1).hdr", {} };

        s_cycle.dayHDR.second   = IBLManager::EnsureIBL(s_cycle.dayHDR.first,   cacheDir);
        s_cycle.dawnHDR.second  = IBLManager::EnsureIBL(s_cycle.dawnHDR.first,  cacheDir);
        s_cycle.nightHDR.second = IBLManager::EnsureIBL(s_cycle.nightHDR.first, cacheDir);

        // cria triple-buffer genérico
        initIBLSet(frontIBL);
        initIBLSet(backIBL);
        initIBLSet(idleIBL);

        frontIBL.brdfLUT = s_cycle.dayHDR.second.brdfLUT;

        computeShader = new ComputeShader("TestCube.comp");
        lerpCubemapShader = new ComputeShader("LerpCubemap.comp");

        init = false;
    }

    bool init = true;
    void Update(float dt) override { 
        return;

        if (init) Init();

        s_cycle.timeOfDay += s_cycle.daySpeed * dt;
        if (s_cycle.timeOfDay >= 24.0f) s_cycle.timeOfDay -= 24.0f;

        float normalized = fmod(s_cycle.timeOfDay, 24.0f) / 24.0f;
        float phase = normalized * 4.0f;
        s_cycle.t = ComputePhaseT(phase);

        UpdateSun(normalized);

        throttleTimer += dt;
        if (throttleTimer >= throttleInterval) {
            throttleTimer = 0.0f;
            if (!jobActive) {
                jobActive = true;
                EnqueueSkyboxJob(); // encadeia os 3
            }
        }
    } 

    float ComputePhaseT(float phase) {
        if (phase < 1.0f) {
            s_cycle.prevIBL = &s_cycle.nightHDR.second; s_cycle.nextIBL = &s_cycle.dawnHDR.second;
            return my_smoothstep(0, 1, phase);
        }
        if (phase < 2.0f) {
            s_cycle.prevIBL = &s_cycle.dawnHDR.second; s_cycle.nextIBL = &s_cycle.dayHDR.second;
            return my_smoothstep(0, 1, phase - 1.0f);
        }
        if (phase < 3.0f) {
            s_cycle.prevIBL = &s_cycle.dayHDR.second; s_cycle.nextIBL = &s_cycle.duskHDR.second;
            return my_smoothstep(0, 1, phase - 2.0f);
        }

        s_cycle.prevIBL = &s_cycle.dawnHDR.second; s_cycle.nextIBL = &s_cycle.dayHDR.second;  //s_cycle.prevIBL = &s_cycle.duskHDR.second; s_cycle.nextIBL = &s_cycle.nightHDR.second;
        return my_smoothstep(0, 1, phase - 3.0f);
    }

    void UpdateSun(float normalized) {
        if (LightPass::currentSun == INVALID_ENTITY) return;
        auto& sunT = GEngine->scene->GetComponent<TransformComponent>(LightPass::currentSun);
        auto& sunL = GEngine->scene->GetComponent<LightComponent>(LightPass::currentSun);

        float sunAngle = glm::radians(-90.0f + 360.0f * normalized);
        sunT.rotation = glm::vec3(sunAngle, 0.0f, 0.0f);

        float sunHeight = glm::clamp(glm::sin(sunAngle), 0.0f, 1.0f);
        sunL.intensity = glm::mix(0.0f, s_cycle.currentExposure * 1.0f, sunHeight);
        //sunL.color = glm::mix(glm::vec3(0.03f), s_cycle.currentSkyTint, glm::clamp(sunHeight * 1.2f, 0.0f, 1.0f));

        s_cycle.currentExposure = glm::mix(s_cycle.exposureNight, s_cycle.exposureDay, s_cycle.t);
        s_cycle.currentSkyTint = glm::mix(s_cycle.nightColor, s_cycle.dayColor, s_cycle.t);
    }

    // retorna o IBL front ativo para render system
    static IBLSet& GetSkyboxFront() { return frontIBL; }

private:
    static inline ComputeShader* computeShader = nullptr;
    static inline ComputeShader* lerpCubemapShader = nullptr;

    static inline IBLSet frontIBL{};
    static inline IBLSet backIBL{};
    static inline IBLSet idleIBL{};
    static inline bool jobActive = false;

    std::string cacheDir = "Engine/Resources/Cache/IBL"; 

    float throttleTimer = 0.0f;
    float throttleInterval = 0.25f;

    // cria cubemaps vazios em cada IBLSet
    void initIBLSet(IBLSet& set) {   
        set.envCubemap = IBLManager::CreateCubemap(512, GL_RGBA16F);
        set.irradiance = IBLManager::CreateCubemap(32,  GL_RGBA16F);
        set.prefilter  = IBLManager::CreateCubemap(128, GL_RGBA16F, 5);
        //set.brdfLUT = 0; // opcional 
    }

    void EnqueueSkyboxJob() {
        CubemapJob* job = new CubemapJob{};
        job->active = true;

        ComputeSyncComponent::Request(
            // GPU dispatch
            [=] {
                DispatchLerp(s_cycle.prevIBL->envCubemap, s_cycle.nextIBL->envCubemap,
                    backIBL.envCubemap, 512, 1);
            },
            // callback encadeia irradiance
            [=] {
                std::swap(frontIBL.envCubemap, backIBL.envCubemap);
                std::swap(backIBL.envCubemap, idleIBL.envCubemap);
                EnqueueIrradianceJob();
            }
        );
    }

    void EnqueueIrradianceJob() {
        ComputeSyncComponent::Request(
            [=] {
                DispatchLerp(s_cycle.prevIBL->irradiance, s_cycle.nextIBL->irradiance,
                    backIBL.irradiance, 32, 1);
            },
            [=] {
                std::swap(frontIBL.irradiance, backIBL.irradiance);
                std::swap(backIBL.irradiance, idleIBL.irradiance);
                EnqueuePrefilterJob();
            }
        );
    }

    void EnqueuePrefilterJob() {
        ComputeSyncComponent::Request(
            [=] {
                DispatchLerp(s_cycle.prevIBL->prefilter, s_cycle.nextIBL->prefilter,
                    backIBL.prefilter, 128, 5);
            },
            [=] {
                std::swap(frontIBL.prefilter, backIBL.prefilter);
                std::swap(backIBL.prefilter, idleIBL.prefilter);

                jobActive = false; // libera o pipeline
            }
        );
    }

    void DispatchLerp(GLuint prevTex, GLuint nextTex, GLuint outTex, int size, int mips) {
        lerpCubemapShader->use();
        lerpCubemapShader->setFloat("lerpFactor", s_cycle.t);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prevTex);
        lerpCubemapShader->setInt("prevCube", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, nextTex);
        lerpCubemapShader->setInt("nextCube", 1);

        for (int mip = 0; mip < mips; ++mip) {
            lerpCubemapShader->setInt("mipLevel", mip);
            glBindImageTexture(2, outTex, mip, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            int groups = (std::max(1, size >> mip) + 7) / 8;
            glDispatchCompute(groups, groups, 6);
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glBindImageTexture(2, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    }

};
