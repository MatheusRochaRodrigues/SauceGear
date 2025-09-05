// === DayNightSystem.h ===
#pragma once
#include "../System.h"
#include "../Scene/SceneECS.h"
#include "../Core/EngineContext.h"
#include "../Components/Transform.h" 
#include "../Graphics/Renderer/PipelinePBR/PBRRenderer.h" 
#include "../Graphics/Renderer/PipelinePBR/IBLManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // para glm::pi etc
#include <glm/gtx/common.hpp>    // smoothstep 


float smoothstep(float edge0, float edge1, float x) {
    float t = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

struct DayNightCycle {
    float timeOfDay = 12.0f;   // 0-24h
    float speed = 0.05f;       // Velocidade do ciclo
    LightComponent sun;           //DirectionalLight

    std::pair<string, IBLSet> dayHDR   = { "Resources/Textures/hdr/tst/Kloppenheim (1).hdr",    {} };   // HDR para o dia            HDRCubemap*
    std::pair<string, IBLSet> dawnHDR  = { "Resources/Textures/hdr/spruit_sunrise_4k.hdr",      {} };   // HDR para o dia            HDRCubemap*
    std::pair<string, IBLSet> nightHDR = { "Resources/Textures/hdr/tst/dikhololo_night_4k.hdr", {} };   // HDR para a noite
    IBLSet currentEnvMap; // Interpolação atual
    float exposure = 1.0f;     // Exposição HDR adaptativa
};

class DayNightSystem : public System {
public:
    float timeOfDay = 12.0f; // 0-24h
    float daySpeed = 0.1f;   // horas por segundo (controla velocidade do ciclo)

    // valores de exposição para cada fase
    float exposureNight = 0.3f;
    float exposureDawn = 0.6f;
    float exposureDay = 1.2f;
    float exposureDusk = 0.7f;

    // cores/IBL para cada fase
    glm::vec3 nightColor = { 0.05f, 0.05f, 0.1f };
    glm::vec3 dawnColor = { 0.9f, 0.5f, 0.3f };
    glm::vec3 dayColor = { 1.0f, 1.0f, 0.95f };
    glm::vec3 duskColor = { 0.9f, 0.4f, 0.2f };
     
    void prepareIBL(IBLSet ibl, string path){ 
        auto& shaders = &PBRPipeline::shaders;
        ibl = IBLManager::EnsureIBL(path, cacheDir,
            shaders.hdrToCube, shaders.irradiance, shaders.prefilter, shaders.brdf,
            iblFBO, iblRBO);
    }

    DayNightSystem() {

        IBLSet dayHDR = { "Resources/Textures/hdr/tst/Kloppenheim (1).hdr",    {} };   // HDR para o dia            HDRCubemap*
        IBLSet dawnHDR = { "Resources/Textures/hdr/spruit_sunrise_4k.hdr",      {} };   // HDR para o dia            HDRCubemap*
        IBLSet nightHDR = { "Resources/Textures/hdr/tst/dikhololo_night_4k.hdr", {} };   // HDR para a noite 
    }

    void Update(float dt) override {
        // avança o horario
        timeOfDay += daySpeed * dt;
        if (timeOfDay >= 24.0f) timeOfDay -= 24.0f;

        float normalized = fmod(timeOfDay, 24.0f) / 24.0f;
        float phase = normalized * 4.0f; // 0-4 (4 fases)
        float t;

        // Definindo fases (em horas)
        // noite: 0-6, amanhecer: 6-8, dia: 8-16, entardecer: 16-18, noite: 18-24
        IBLSet crtHDR;     //HDRCubemap,  Irradiance, PreFilter
        IBLSet nextHDR;     

        glm::vec3 skyColor;
        float exposure;

        if (phase < 1.0f) { // Noite  -> Amanhecer
            t = phase;
            skyColor = glm::mix(nightColor, dawnColor, smoothstep(0.0f, 1.0f, t));
            exposure = glm::mix(exposureNight, exposureDawn, t);
            crtHDR = nightHDR;
            nextHDR = dawnHDR;
        }
        else if (phase < 2.0f) { // Amanhecer ->  Dia
            t = phase - 1.0f;
            skyColor = glm::mix(dawnColor, dayColor, smoothstep(0.0f, 1.0f, t));
            exposure = glm::mix(exposureDawn, exposureDay, t);
            crtHDR = ;
            nextHDR = ;
        }
        else if (phase < 3.0f) { // Dia ->  Entardecer
            t = phase - 2.0f;
            skyColor = glm::mix(dayColor, duskColor, smoothstep(0.0f, 1.0f, t));
            exposure = glm::mix(exposureDay, exposureDusk, t);
            crtHDR = ;
            nextHDR = ;
        }
        else { // Entardecer -> Noite
            t = phase - 3.0f;
            skyColor = glm::mix(duskColor, nightColor, smoothstep(0.0f, 1.0f, t));
            exposure = glm::mix(exposureDusk, exposureNight, t);
            crtHDR = ;
            nextHDR = ;
        }

        // atualiza luz direcional (sol)
        if (LightSystem::currentSun != INVALID_ENTITY) {
            auto& sunTransform = GEngine->scene->GetComponent<Transform>(LightSystem::currentSun);
            auto& sunLight = GEngine->scene->GetComponent<LightComponent>(LightSystem::currentSun);

            float sunAngle = glm::radians(-90.0f + 360.0f * normalized);
            float moonAngle = glm::radians(-90.0f + 360.0f * (normalized + 0.5f)); // +0.5 - metade do ciclo oposto

            sunTransform.rotation = glm::vec3(sunAngle, 0.0f, 0.0f);
            //moonTransform.rotation = glm::vec3(moonAngle, 0.0f, 0.0f);

            sunLight.color = skyColor;
            sunLight.intensity = exposure;
        }

        // atualiza PostProcess (HDR exposure)
        GEngine->postProcess->SetExposure(exposure);

        // atualiza Skybox/IBL
        UpdateSkyboxIBL(skyColor);
    }

private:
    void UpdateSkyboxIBL(const glm::vec3& skyColor) {
        // Aqui voce pode gerar gradiente procedural ou 
        // interpolar entre diferentes cubemaps HDR.
        // Exemplo simples: enviar cor pro shader de skybox.

        // Interpolação GPU (compute shader)
        ComputeLerpCubemap(*prevHDR, *nextHDR, t, cycle.currentSkybox);     // Interpola cubemaps filtrados para reflexos PBR
        ComputeLerpCubemap(*prevIrr, *nextIrr, t, cycle.currentIrradiance);
        ComputeLerpCubemap(*prevPref, *nextPref, t, cycle.currentPrefilter);

        auto skyShader = GEngine->renderer->skyboxShader;
        skyShader->use();
        skyShader->setVec3("skyColor", skyColor);
    }
      
    //Se não houver grandes mudanças, pode atualizar a cada N frames
    Cubemap LerpCubemap(const Cubemap& night, const Cubemap& day, float t) {
        Cubemap result;
        result.width = day.width;
        result.height = day.height;
        result.mipLevels = day.mipLevels;
        result.format = day.format;

        glGenTextures(1, &result.id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, result.id);

        for (int mip = 0; mip < day.mipLevels; ++mip) {
            int mipWidth = day.width >> mip;
            int mipHeight = day.height >> mip;

            for (int face = 0; face < 6; ++face) {
                // Obter dados dos cubemaps de dia e noite
                std::vector<float> dataDay(mipWidth * mipHeight * 3);
                std::vector<float> dataNight(mipWidth * mipHeight * 3);
                day.GetFaceData(face, mip, dataDay.data());         //deve retornar os pixels RGB float do cubemap filtrado
                night.GetFaceData(face, mip, dataNight.data());

                // Interpolação pixel a pixel
                std::vector<float> dataLerp(dataDay.size());
                for (size_t i = 0; i < dataDay.size(); ++i)
                    dataLerp[i] = glm::mix(dataNight[i], dataDay[i], t);

                // Upload do mip interpolado
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip,
                    GL_RGB16F, mipWidth, mipHeight, 0, GL_RGB, GL_FLOAT, dataLerp.data());
            }
        }

        // Configurações padrão
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return result;
    }

     
    std::string cacheDir = "Resources/Cache/IBL";
};




//float t = glm::clamp((sin(cycle.timeOfDay / 24.0f * glm::pi<float>() * 2.0f) + 1.0f) / 2.0f, 0.0f, 1.0f);


//struct DirectionalLight {
//    glm::vec3 direction;
//    glm::vec3 color;
//    float intensity;
//};


// rotação da luz = posicao do sol
//float angle = normalized * glm::two_pi<float>();
//sunTransform.rotation = glm::vec3(
//    glm::radians(-90.0f + 180.0f * normalized), // sobe/desce sol
//    0.0f, 0.0f );


// 2. Sol: direção e cor
//float sunAngle = glm::radians(cycle.timeOfDay / 24.0f * 360.0f);
//cycle.sun.direction = glm::normalize(glm::vec3(cos(sunAngle), sin(sunAngle), 0.0f));




//// Atualiza sol
//float sunAngle = glm::radians(cycle.timeOfDay / 24.0f * 360.0f);
//cycle.sun.direction = glm::normalize(glm::vec3(cos(sunAngle), sin(sunAngle), 0.0f));
//cycle.sun.intensity = glm::clamp(glm::dot(cycle.sun.direction, glm::vec3(0, 1, 0)), 0.0f, 1.0f);
//cycle.sun.color = glm::mix(glm::vec3(0.05f), glm::vec3(1.0f), cycle.sun.intensity);
//
//// Exposição adaptativa
//cycle.exposure = glm::mix(0.2f, 1.5f, cycle.sun.intensity);




//class DayNightSystem : public System {
//public:
//    float timeOfDay = 0.0f; // 0.0 -> meia-noite, 0.5 -> meio-dia, 1.0 -> meia-noite
//    float daySpeed = 0.01f; // velocidade do ciclo
//
//
//    void Update(float deltaTime) override {
//
//        timeOfDay += deltaTime * daySpeed;
//        if (timeOfDay > 1.0f) timeOfDay -= 1.0f;
//
//        auto all = GEngine->scene->GetEntitiesWith<LightComponent, Transform>();
//
//        for (Entity e : all) {
//            auto& light = GEngine->scene->GetComponent<LightComponent>(e);
//            auto& transform = GEngine->scene->GetComponent<Transform>(e);
//
//            if (light.type == ShadowType::Directional) {
//                // Rotação básica do sol em torno de X
//                float angle = (timeOfDay * 360.0f) - 90.0f;
//                transform.rotation = glm::vec3(angle, 0.0f, 0.0f);
//
//                // Ajuste de intensidade conforme altura
//                float sunHeight = glm::sin(glm::radians(angle));
//                light.intensity = glm::clamp(sunHeight, 0.0f, 1.0f);
//
//                // Cor do sol simples (amarelado no pôr, branco ao meio-dia)
//                if (sunHeight > 0.0f) {
//                    light.color = glm::mix(glm::vec3(1.0f, 0.6f, 0.3f),
//                        glm::vec3(1.0f, 1.0f, 0.9f),
//                        sunHeight);
//                }
//                else {
//                    light.color = glm::vec3(0.05f, 0.05f, 0.2f); // noite
//                }
//            }
//        }
//    }
//
//
//};




// 4. Interpolação suave IBL e skybox
//float t = glm::clamp((sin(cycle.timeOfDay / 24.0f * glm::pi<float>() * 2.0f) + 1.0f) / 2.0f, 0.0f, 1.0f);




//#include "ISystem.h"
//#include "../ECS/SceneECS.h"
//#include "../Components/TransformComponent.h"
//#include "../Components/LightComponent.h"





/*
float GetPhaseFactor(float time, float start, float end) {
    float t = (time - start) / (end - start);
    return glm::clamp(t, 0.0f, 1.0f);
}

void UpdateDayNightWithDawnDusk(DayNightCycle& cycle, float deltaTime) {
    cycle.timeOfDay += deltaTime * cycle.speed;
    if (cycle.timeOfDay > 24.0f) cycle.timeOfDay -= 24.0f;

    // Definindo fases (em horas)
    // noite: 0-6, amanhecer: 6-8, dia: 8-16, entardecer: 16-18, noite: 18-24
    HDRCubemap* prevHDR;
    HDRCubemap* nextHDR;
    Cubemap* prevIrr;
    Cubemap* nextIrr;
    Cubemap* prevPref;
    Cubemap* nextPref;
    float t = 0.0f;

    if (cycle.timeOfDay < 6.0f) {
        prevHDR = cycle.nightHDR; nextHDR = cycle.dawnHDR;
        prevIrr = &cycle.irradianceNight; nextIrr = &cycle.irradianceDawn;
        prevPref = &cycle.prefilterNight; nextPref = &cycle.prefilterDawn;
        t = GetPhaseFactor(cycle.timeOfDay, 0.0f, 6.0f);
    }
    else if (cycle.timeOfDay < 8.0f) {
        prevHDR = cycle.dawnHDR; nextHDR = cycle.dayHDR;
        prevIrr = &cycle.irradianceDawn; nextIrr = &cycle.irradianceDay;
        prevPref = &cycle.prefilterDawn; nextPref = &cycle.prefilterDay;
        t = GetPhaseFactor(cycle.timeOfDay, 6.0f, 8.0f);
    }
    else if (cycle.timeOfDay < 16.0f) {
        prevHDR = cycle.dayHDR; nextHDR = cycle.duskHDR;
        prevIrr = &cycle.irradianceDay; nextIrr = &cycle.irradianceDusk;
        prevPref = &cycle.prefilterDay; nextPref = &cycle.prefilterDusk;
        t = GetPhaseFactor(cycle.timeOfDay, 8.0f, 16.0f);
    }
    else if (cycle.timeOfDay < 18.0f) {
        prevHDR = cycle.duskHDR; nextHDR = cycle.nightHDR;
        prevIrr = &cycle.irradianceDusk; nextIrr = &cycle.irradianceNight;
        prevPref = &cycle.prefilterDusk; nextPref = &cycle.prefilterNight;
        t = GetPhaseFactor(cycle.timeOfDay, 16.0f, 18.0f);
    }
    else {
        prevHDR = cycle.nightHDR; nextHDR = cycle.dawnHDR;
        prevIrr = &cycle.irradianceNight; nextIrr = &cycle.irradianceDawn;
        prevPref = &cycle.prefilterNight; nextPref = &cycle.prefilterDawn;
        t = GetPhaseFactor(cycle.timeOfDay, 18.0f, 24.0f);
    }

    // Atualiza sol
    float sunAngle = glm::radians(cycle.timeOfDay / 24.0f * 360.0f);
    cycle.sun.direction = glm::normalize(glm::vec3(cos(sunAngle), sin(sunAngle), 0.0f));
    cycle.sun.intensity = glm::clamp(glm::dot(cycle.sun.direction, glm::vec3(0, 1, 0)), 0.0f, 1.0f);
    cycle.sun.color = glm::mix(glm::vec3(0.05f), glm::vec3(1.0f), cycle.sun.intensity);

    // Exposição adaptativa
    cycle.exposure = glm::mix(0.2f, 1.5f, cycle.sun.intensity);

    // Interpolação GPU (compute shader)
    ComputeLerpCubemap(*prevHDR, *nextHDR, t, cycle.currentSkybox);
    ComputeLerpCubemap(*prevIrr, *nextIrr, t, cycle.currentIrradiance);
    ComputeLerpCubemap(*prevPref, *nextPref, t, cycle.currentPrefilter);
}
*/