#pragma once

#include "../System.h"
#include "../Scene/SceneECS.h"
#include "../Graphics/ComputeShader.h"
#include "../Core/EngineContext.h"
#include "../Components/Transform.h"
#include "../Components/LightComponent.h"
#include "../../Graphics/Renderer/PipelinePBR/PBRRenderer.h"
#include "../../Graphics/Renderer/PipelinePBR/IBLManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/common.hpp>
#include <string>

using std::string;

// helper smoothstep (se não tiver disponível)
static float my_smoothstep(float edge0, float edge1, float x) {
    float t = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

/*
  Assunções sobre IBLSet (ajuste caso seja diferente no teu código):
  struct IBLSet {
      GLuint skybox;     // cubemap HDR (GL_TEXTURE_CUBE_MAP)
      GLuint irradiance; // irradiance cubemap (GL_TEXTURE_CUBE_MAP)
      GLuint prefilter;  // prefiltered cubemap (GL_TEXTURE_CUBE_MAP)
      int   width;       // base width (ex: 512)
      int   mipLevels;   // número de mipmaps do prefilter
  };
*/

struct DayNightCycle {
    // valores configuráveis:
    float timeOfDay = 12.0f;   // 0..24
    float daySpeed = 0.05f;   // horas por segundo - controla velocidade do ciclo

    // exposições por fase
    float exposureNight = 0.25f;
    float exposureDawn = 0.6f;
    float exposureDay = 1.3f;
    float exposureDusk = 0.7f;

    // cores por fase (usadas para cor do sol e sky tint)
    glm::vec3 nightColor = { 0.05f, 0.05f, 0.1f };
    glm::vec3 dawnColor = { 0.9f, 0.45f, 0.3f };
    glm::vec3 dayColor = { 1.0f, 1.0f, 0.95f };
    glm::vec3 duskColor = { 0.9f, 0.4f, 0.2f };

    // HDR resources para cada fase (par caminho + IBLSet preenchido)
    std::pair<string, IBLSet> nightHDR = { "Resources/Textures/hdr/tst/dikhololo_night_4k.hdr", {} };
    std::pair<string, IBLSet> dawnHDR = { "Resources/Textures/hdr/spruit_sunrise_4k.hdr",     {} };
    std::pair<string, IBLSet> dayHDR = { "Resources/Textures/hdr/tst/Kloppenheim (1).hdr",   {} };

    std::pair<string, IBLSet> duskHDR = { "Resources/Textures/hdr/tst/rogland_moonlit_night_4k.hdr",  {} }; // ajuste caminho
    //std::pair<string, IBLSet> duskHDR = { "Resources/Textures/hdr/tst/dusk.hdr",             {} }; // ajuste caminho

    // resultado atual (guardado para leitura pelos shaders)
    //IBLSet currentIBL{};   // interpolated sky/irradiance/prefilter (GPU-written cubemaps)
    //float   currentExposure = 1.0f;
    //glm::vec3 currentSkyTint = { 1.0f, 1.0f, 1.0f };
};


class DayNightSystem : public System {
public:
    // ciclo global estático: shaders PBR/IBL vão ler DayNightSystem::s_currentIBL / s_currentExposure
    static inline DayNightCycle s_cycle;

    PBRShaders shaders;
    DayNightSystem() {
        // pre-generate IBLs: transforma HDR -> cubemap, irradiance, prefilter, brdf LUT etc.
        // o EnsureIBL preenche um IBLSet (assumi a API: EnsureIBL(path, cacheDir, shaders...))
        //auto shaders = PBRPipeline::shaders; // já existente na tua engine
        s_cycle.dayHDR.second   = IBLManager::EnsureIBL(s_cycle.dayHDR.first, cacheDir, shaders.hdrToCube, shaders.irradiance, shaders.prefilter, shaders.brdf, 0, 0);
        s_cycle.dawnHDR.second  = IBLManager::EnsureIBL(s_cycle.dawnHDR.first, cacheDir, shaders.hdrToCube, shaders.irradiance, shaders.prefilter, shaders.brdf, 0, 0);
        s_cycle.nightHDR.second = IBLManager::EnsureIBL(s_cycle.nightHDR.first, cacheDir, shaders.hdrToCube, shaders.irradiance, shaders.prefilter, shaders.brdf, 0, 0);
        s_cycle.duskHDR.second  = IBLManager::EnsureIBL(s_cycle.duskHDR.first, cacheDir, shaders.hdrToCube, shaders.irradiance, shaders.prefilter, shaders.brdf, 0, 0);

        // cria cubemap destino (currentIBL) com mesma resolução/mips do day (ou maior)
        // Aqui lembre-se de criar textures GL_TEXTURE_CUBE_MAP para s_cycle.currentIBL.skybox/irradiance/prefilter
        // Ex.: currentIBL.skybox = CreateEmptyCubemap(day.width, day.mipLevels, GL_RGBA16F);
        CreateEmptyCurrentIBL();
         
        computeShader = new ComputeShader("TestCube.comp");
        lerpCubemapShader = new ComputeShader("LerpCubemap.comp");
    }

    void Update(float dt) override {
        // avança o tempo
        s_cycle.timeOfDay += s_cycle.daySpeed * dt;
        if (s_cycle.timeOfDay >= 24.0f) s_cycle.timeOfDay -= 24.0f;

        float normalized = fmod(s_cycle.timeOfDay, 24.0f) / 24.0f; // [0,1)
        float phase = normalized * 4.0f; // quatro segmentos
        float t = 0.0f;

        // refs para prev e next IBLSet
        IBLSet* prevIBL = nullptr;
        IBLSet* nextIBL = nullptr;
        glm::vec3 skyColor;
        float exposure; 

        if (phase < 1.0f) { // 0..1 -> Noite -> Amanhecer (0: midnight)
            t = my_smoothstep(0.0f, 1.0f, phase - 0.0f);
            skyColor = glm::mix(s_cycle.nightColor, s_cycle.dawnColor, t);
            exposure = glm::mix(s_cycle.exposureNight, s_cycle.exposureDawn, t);
            prevIBL = &s_cycle.nightHDR.second;
            nextIBL = &s_cycle.dawnHDR.second;
        }
        else if (phase < 2.0f) { // 1..2 -> Amanhecer -> Dia
            t = my_smoothstep(0.0f, 1.0f, phase - 1.0f);
            skyColor = glm::mix(s_cycle.dawnColor, s_cycle.dayColor, t);
            exposure = glm::mix(s_cycle.exposureDawn, s_cycle.exposureDay, t);
            prevIBL = &s_cycle.dawnHDR.second;
            nextIBL = &s_cycle.dayHDR.second;
        }
        else if (phase < 3.0f) { // 2..3 -> Dia -> Entardecer
            t = my_smoothstep(0.0f, 1.0f, phase - 2.0f);
            skyColor = glm::mix(s_cycle.dayColor, s_cycle.duskColor, t);
            exposure = glm::mix(s_cycle.exposureDay, s_cycle.exposureDusk, t);
            prevIBL = &s_cycle.dayHDR.second;
            nextIBL = &s_cycle.duskHDR.second;
        }
        else { // 3..4 -> Entardecer -> Noite
            t = my_smoothstep(0.0f, 1.0f, phase - 3.0f);
            skyColor = glm::mix(s_cycle.duskColor, s_cycle.nightColor, t);
            exposure = glm::mix(s_cycle.exposureDusk, s_cycle.exposureNight, t);
            prevIBL = &s_cycle.duskHDR.second;
            nextIBL = &s_cycle.nightHDR.second;
        }

        // Atualiza luz direcional (sol) — se existir
        if (LightSystem::currentSun != INVALID_ENTITY) {
            auto& sunTransform = GEngine->scene->GetComponent<Transform>(LightSystem::currentSun);
            auto& sunLight = GEngine->scene->GetComponent<LightComponent>(LightSystem::currentSun);

            float sunAngle = glm::radians(-90.0f + 360.0f * normalized); // -90 start so sunrise around 0.25
            // rotaciona eixo X (ajuste conforme teu sistema de transform)
            sunTransform.rotation = glm::vec3(sunAngle, 0.0f, 0.0f);

            // intensidade e cor do sol baseados no exposure/skyColor (pode ajustar curva)
            float sunHeight = glm::clamp(glm::sin(sunAngle), 0.0f, 1.0f); // 0 = abaixo do horizonte
            sunLight.intensity = glm::mix(0.0f, exposure * 1.0f, sunHeight); // intensidade real com base no solHeight
            // mistura da cor do sol com um pouco de brilho
            sunLight.color = glm::mix(glm::vec3(0.03f), skyColor, glm::clamp(sunHeight * 1.2f, 0.0f, 1.0f));
        }

        // Atualiza exposição e sky tint globais (estáticos para shaders)
        currentExposure = exposure;
        currentSkyTint = skyColor;

        // Interpola IBLs na GPU (compute shader)
        // prevIBL / nextIBL apontam para IBLSet já gerados por EnsureIBL
         
        if (prevIBL && nextIBL) {
            //std::cout << "Time23 " << currentIBL.envCubemap << std::endl;
            // skybox (HDR cubemap)
            GPU_LerpCubemap(prevIBL->envCubemap, nextIBL->envCubemap, backIBL.envCubemap, 
                t, 512, 1);

            // irradiance (diffuse)
            GPU_LerpCubemap(prevIBL->irradiance, nextIBL->irradiance, backIBL.irradiance,
                t, 32, 1);

            // prefilter (specular) -> cada mip level já será processado pelo compute shader
            GPU_LerpCubemap(prevIBL->prefilter, nextIBL->prefilter, backIBL.prefilter,
                t, 128, 5);
        }
         
        // atualiza PostProcess / renderer com exposure e IBL
        //GEngine->postProcess->SetExposure(s_cycle.currentExposure);
         
    }

    // -----------------------
    // Helpers / GPU Lerp
    // -----------------------
    // Envia os cubemaps prev/next (GL_TEXTURE_CUBE_MAP) e escreve out (também GL_TEXTURE_CUBE_MAP) via compute shader.
    // prevWidth/mipCount referem-se ao prev cubemap (assumi ambos com mesma dims/mips).
    void GPU_LerpCubemap(GLuint prevTex, GLuint nextTex, GLuint outTex, float t, int prevWidth, int mipCount) {
        // bind shader
        lerpCubemapShader->use();
        lerpCubemapShader->setFloat("lerpFactor", t);

        // ativa amostras 0 e 1
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prevTex);
        lerpCubemapShader->setInt("prevCube", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, nextTex);
        lerpCubemapShader->setInt("nextCube", 1);

        // bind imagem de saída (imageCube) - outTex já deve existir e ter mipmaps alocados
        for (int mip = 0; mip < mipCount; ++mip) {
            lerpCubemapShader->setInt("mipLevel", mip);
            glBindImageTexture(2, outTex, mip, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

            // Dispatch: cubemap é normalmente quadrado
            int size = std::max(1, prevWidth >> mip);       //size = prevWidth / 2^mip

            //int groupsX = (size + 7) / 8;
            //int groupsY = (size + 3) / 4; // teu compute shader usa local_size_x=8, local_size_y=4
            //// 6 faces: usamos z = 6
            //glDispatchCompute(groupsX, groupsY, 6); 

            int groups = (size + 7) / 8; // local_size = 8
            glDispatchCompute(groups, groups, 6); 

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        }

        // unbind
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glBindImageTexture(2, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    }

    // Cria as textures vazias para s_cycle.currentIBL (skybox/irradiance/prefilter)
    void CreateEmptyCurrentIBL() {   
        backIBL = IBLManager::CreateEmptyIBL();
        frontIBL = IBLManager::CreateEmptyIBL(); 
    }

    // -----------------------
    // dados públicos para o pipeline ler
    // -----------------------
    // interpolated sky/irradiance/prefilter (GPU-written cubemaps)
    static inline IBLSet frontIBL{};  
    static inline float  currentExposure = 1.0f;  // exposição adaptativa atual
    static inline glm::vec3 currentSkyTint = { 1.0f, 1.0f, 1.0f }; 


    static GLuint CreateTestCubemap(int size) {
        static GLuint tex = 0;  

        if (tex == 0) { 
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

            for (int face = 0; face < 6; ++face) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
                    GL_RGBA16F, size, size, 0, GL_RGBA, GL_FLOAT, nullptr);
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
             
        }

        int updateIndex = GEngine->time->GetFrameCount() % 20;
        if (updateIndex == 0) { 
            computeShader->use();
            glBindImageTexture(0, tex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

            int groups = (size + 7) / 8;
            glDispatchCompute(groups, groups, 6); // 6 faces
            //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            // garante que TODAS as escritas estejam visíveis antes de usar como sampler
            //glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        }

        return tex;
    }


private:
    static inline IBLSet backIBL{};       // cubemaps interpolados (GPU-written)
    static inline ComputeShader* computeShader ; // shader compute (deve ser inicializado com o programa .comp)
    
    static inline ComputeShader* lerpCubemapShader; // shader compute (deve ser inicializado com o programa .comp)
    GLuint iblFBO = 0, iblRBO = 0;
    std::string cacheDir = "Resources/Cache/IBL";
};
















// Cria as textures vazias para s_cycle.currentIBL (skybox/irradiance/prefilter)
//void CreateEmptyCurrentIBL() {
//    // Cria cubemaps com as mesmas specs do dayHDR (assumo dayHDR já preenchido)
//    IBLSet& ref = s_cycle.dayHDR.second;
//    if (ref.envCubemap == 0) return; // fallback se algo não carregado ainda
//
//    // helper para criar
//    auto CreateEmptyCube = [&](int width, int mipLevels)->GLuint {
//        GLuint tex;
//        glGenTextures(1, &tex);
//        glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
//        for (int face = 0; face < 6; ++face) {
//            for (int mip = 0; mip < mipLevels; ++mip) {
//                int w = std::max(1, width >> mip);
//                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, GL_RGBA16F, w, w, 0, GL_RGBA, GL_FLOAT, nullptr);
//            }
//        }
//        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
//        return tex;
//        };
//
//    int w = ref.width;
//    int mips = ref.mipLevels;
//    //skybox
//    s_cycle.currentIBL.envCubemap = CreateEmptyCube(w, 1); // skybox normalmente só level0
//    s_cycle.currentIBL.irradiance = CreateEmptyCube(w / 4, 1); // irradiance costuma ser small
//    s_cycle.currentIBL.prefilter = CreateEmptyCube(w, mips);
//}