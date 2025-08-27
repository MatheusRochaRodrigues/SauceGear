// === DayNightSystem.h ===
#pragma once
#include "../System.h"
#include "../SceneECS.h"
#include "../Core/EngineContext.h"
#include "../Components/Transform.h"
#include "../Components/LightComponent.h"
#include "../Graphics/Renderer/PipelinePBR/IBLManager.h"

//struct DirectionalLight {
//    glm::vec3 direction;
//    glm::vec3 color;
//    float intensity;
//};



struct DayNightCycle {
    float timeOfDay = 12.0f;   // 0-24h
    float speed = 0.05f;       // Velocidade do ciclo
    LightComponent sun;           //DirectionalLight

    std::pair<string, IBLSet> dayHDR   = { "Resources/Textures/hdr/tst/Kloppenheim (1).hdr", {} };   // HDR para o dia            HDRCubemap*
    std::pair<string, IBLSet> dawnHDR  = { "Resources/Textures/hdr/spruit_sunrise_4k.hdr", {} };   // HDR para o dia            HDRCubemap*
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

    void Update(float dt) override {
        // avança o horario
        timeOfDay += daySpeed * dt;
        if (timeOfDay >= 24.0f) timeOfDay -= 24.0f;

        float normalized = fmod(timeOfDay, 24.0f) / 24.0f;
        float phase = normalized * 4.0f; // 0-4 (4 fases)
        float t;

        glm::vec3 skyColor;
        float exposure;

        if (phase < 1.0f) { // Noite  -> Amanhecer
            t = phase;
            skyColor = glm::mix(nightColor, dawnColor, smoothstep(0.0f, 1.0f, t));
            exposure = glm::mix(exposureNight, exposureDawn, t);
        }
        else if (phase < 2.0f) { // Amanhecer ->  Dia
            t = phase - 1.0f;
            skyColor = glm::mix(dawnColor, dayColor, smoothstep(0.0f, 1.0f, t));
            exposure = glm::mix(exposureDawn, exposureDay, t);
        }
        else if (phase < 3.0f) { // Dia ->  Entardecer
            t = phase - 2.0f;
            skyColor = glm::mix(dayColor, duskColor, smoothstep(0.0f, 1.0f, t));
            exposure = glm::mix(exposureDay, exposureDusk, t);
        }
        else { // Entardecer -> Noite
            t = phase - 3.0f;
            skyColor = glm::mix(duskColor, nightColor, smoothstep(0.0f, 1.0f, t));
            exposure = glm::mix(exposureDusk, exposureNight, t);
        }

        // atualiza luz direcional (sol)
        if (LightSystem::currentSun != INVALID_ENTITY) {
            auto& sunTransform = GEngine->scene->GetComponent<Transform>(LightSystem::currentSun);
            auto& sunLight = GEngine->scene->GetComponent<LightComponent>(LightSystem::currentSun);

            // rotação da luz = posição do sol
            float angle = normalized * glm::two_pi<float>();
            sunTransform.rotation = glm::vec3(
                glm::radians(-90.0f + 180.0f * normalized), // sobe/desce sol
                0.0f,
                0.0f
            );

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
        auto skyShader = GEngine->renderer->skyboxShader;
        skyShader->use();
        skyShader->setVec3("skyColor", skyColor);
    }
};



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







//#include "ISystem.h"
//#include "../ECS/SceneECS.h"
//#include "../Components/TransformComponent.h"
//#include "../Components/LightComponent.h"