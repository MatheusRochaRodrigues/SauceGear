//#pragma once
#include "../../ECS/Components/ComponentsHelper.h"
#include "../../Scene/SceneECS.h"
#include "../../ECS/System.h"
#include "../../Graphics/Renderer/PipelinePBR/IBLManager.h"
#include "../../Graphics/Renderer/PipelinePBR/PBRRenderer.h"
#include "../../Graphics/ComputeShader.h"
#include "../../Core/EngineContext.h" 

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

class DayNightSystem : public System {
public:
    static inline DayNightCycle s_cycle;
    static inline IBLSet frontIBL{};

    float throttleTimer = 0.0f;
    float throttleInterval = 2.0f; // 0.1f segundos entre interpolação

    struct CubemapJob {
        GLuint prevTex;
        GLuint nextTex;
        GLuint outTex;
        int size;
        int mipCount;
        bool active = false;
    };

    CubemapJob skyboxJob;
    CubemapJob irradianceJob;
    CubemapJob prefilterJob;

    DayNightSystem() {
        // HDR paths
        s_cycle.nightHDR = { "Resources/Textures/hdr/tst/dikhololo_night_4k.hdr", {} };
        s_cycle.dawnHDR = { "Resources/Textures/hdr/spruit_sunrise_4k.hdr", {} };
        s_cycle.dayHDR = { "Resources/Textures/hdr/tst/Kloppenheim (1).hdr", {} };
        //s_cycle.duskHDR = { "Resources/Textures/hdr/tst/rogland_moonlit_night_4k.hdr", {} };

        // pré-processa HDR → cubemap, irradiance, prefilter
        s_cycle.dayHDR.second = IBLManager::EnsureIBL(s_cycle.dayHDR.first, cacheDir, shaders.hdrToCube, shaders.irradiance, shaders.prefilter, shaders.brdf, 0, 0);
        s_cycle.dawnHDR.second = IBLManager::EnsureIBL(s_cycle.dawnHDR.first, cacheDir, shaders.hdrToCube, shaders.irradiance, shaders.prefilter, shaders.brdf, 0, 0);
        s_cycle.nightHDR.second = IBLManager::EnsureIBL(s_cycle.nightHDR.first, cacheDir, shaders.hdrToCube, shaders.irradiance, shaders.prefilter, shaders.brdf, 0, 0);
        //s_cycle.duskHDR.second = IBLManager::EnsureIBL(s_cycle.duskHDR.first, cacheDir, shaders.hdrToCube, shaders.irradiance, shaders.prefilter, shaders.brdf, 0, 0);

        backIBL = IBLManager::CreateEmptyIBL();
        frontIBL = IBLManager::CreateEmptyIBL();

        computeShader = new ComputeShader("TestCube.comp");
        lerpCubemapShader = new ComputeShader("LerpCubemap.comp");
    }

    void Update(float dt) override {
        s_cycle.timeOfDay += s_cycle.daySpeed * dt;
        if (s_cycle.timeOfDay >= 24.0f) s_cycle.timeOfDay -= 24.0f;

        float normalized = fmod(s_cycle.timeOfDay, 24.0f) / 24.0f;
        float phase = normalized * 4.0f;
        s_cycle.t = ComputePhaseT(phase);

        // Nunca chame StartCubemapJob() dentro do draw. Faça no Update
        DayNightSystem::CreateTestCubemap3(512);

        UpdateSun(normalized);

        throttleTimer += dt;
        if (throttleTimer >= throttleInterval) {
            throttleTimer = 0.0f;
            if (!skyboxJob.active) EnqueueCubemapJob(skyboxJob, s_cycle.prevIBL->envCubemap, s_cycle.nextIBL->envCubemap, backIBL.envCubemap, 512, 1);
            if (!irradianceJob.active) EnqueueCubemapJob(irradianceJob, s_cycle.prevIBL->irradiance, s_cycle.nextIBL->irradiance, backIBL.irradiance, 32, 1);
            if (!prefilterJob.active) EnqueueCubemapJob(prefilterJob, s_cycle.prevIBL->prefilter, s_cycle.nextIBL->prefilter, backIBL.prefilter, 128, 5);
        }
    }

private:
    IBLSet backIBL{};

    static inline ComputeShader* computeShader = nullptr;
    static inline ComputeShader* lerpCubemapShader = nullptr;

    std::string cacheDir = "Resources/Cache/IBL";
    PBRShaders shaders;

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
        if (LightSystem::currentSun == INVALID_ENTITY) return;
        auto& sunT = GEngine->scene->GetComponent<Transform>(LightSystem::currentSun);
        auto& sunL = GEngine->scene->GetComponent<LightComponent>(LightSystem::currentSun);

        float sunAngle = glm::radians(-90.0f + 360.0f * normalized);
        sunT.rotation = glm::vec3(sunAngle, 0.0f, 0.0f);

        float sunHeight = glm::clamp(glm::sin(sunAngle), 0.0f, 1.0f);
        sunL.intensity = glm::mix(0.0f, s_cycle.currentExposure * 1.0f, sunHeight);
        sunL.color = glm::mix(glm::vec3(0.03f), s_cycle.currentSkyTint, glm::clamp(sunHeight * 1.2f, 0.0f, 1.0f));

        s_cycle.currentExposure = glm::mix(s_cycle.exposureNight, s_cycle.exposureDay, s_cycle.t);
        s_cycle.currentSkyTint = glm::mix(s_cycle.nightColor, s_cycle.dayColor, s_cycle.t);
    }

    void EnqueueCubemapJob(CubemapJob& job, GLuint prevTex, GLuint nextTex, GLuint outTex, int size, int mips) {
        if (job.active) return;
        job.active = true;

        // Captura ponteiro seguro pro job
        CubemapJob* jobPtr = &job;

        // Usa o novo padrão: Request(gpuWork, onComplete)
        ComputeSyncComponent::Request(
            // 1) GPU work (dispatch do lerp)
            [=] {
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

                // limpeza
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                glBindImageTexture(2, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            },

            // 2) Callback após GPU terminar
            [this, jobPtr, size]() {
                if (jobPtr == &skyboxJob) {
                    glCopyImageSubData(backIBL.envCubemap, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                        frontIBL.envCubemap, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                        size, size, 6);
                }
                else if (jobPtr == &irradianceJob) {
                    glCopyImageSubData(backIBL.irradiance, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                        frontIBL.irradiance, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                        size, size, 6);
                }
                else if (jobPtr == &prefilterJob) {
                    glCopyImageSubData(backIBL.prefilter, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                        frontIBL.prefilter, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                        size, size, 6);
                }

                jobPtr->active = false; // libera o job
            }
        );
    }



    void GPU_LerpCubemap(GLuint prevTex, GLuint nextTex, GLuint outTex, float t, int prevWidth, int mipCount) {
        lerpCubemapShader->use();
        lerpCubemapShader->setFloat("lerpFactor", t);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP, prevTex); lerpCubemapShader->setInt("prevCube", 0);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_CUBE_MAP, nextTex); lerpCubemapShader->setInt("nextCube", 1);

        for (int mip = 0; mip < mipCount; ++mip) {
            lerpCubemapShader->setInt("mipLevel", mip);
            glBindImageTexture(2, outTex, mip, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            int groups = (std::max(1, prevWidth >> mip) + 7) / 8;
            glDispatchCompute(groups, groups, 6);
        }
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glBindImageTexture(2, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    }

    static void CopyCubemapSync(GLuint src, GLuint dst, int size, int mipCount = 1) {
        for (int mip = 0; mip < mipCount; ++mip) {
            glCopyImageSubData(
                src, GL_TEXTURE_CUBE_MAP, mip, 0, 0, 0,
                dst, GL_TEXTURE_CUBE_MAP, mip, 0, 0, 0,
                size >> mip, size >> mip, 6
            );
        }

        // Criar fence
        GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        if (!fence) return;

        // Espera síncrona (bloqueante) até a GPU terminar
        GLenum wait = GL_UNSIGNALED;
        while (wait != GL_ALREADY_SIGNALED && wait != GL_CONDITION_SATISFIED) {
            wait = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000); // timeout 1s
        }

        glDeleteSync(fence);
    } 

public:
    static inline GLuint texFront = 0;
    static GLuint CreateTestCubemap3(int size) {
        static bool jobActive = false;
        static GLuint texBack = 0;
        static GLuint texIdle = 0;

        auto initTexture = [&](GLuint& tex) {
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
            };

        initTexture(texFront);
        initTexture(texBack);
        initTexture(texIdle);

        if (!jobActive) {
            jobActive = true;

            // Capture por referência PARA que a callback realmente altere as variáveis static
            ComputeSyncComponent::Request(
                // gpuWork - captura por referência também é ok aqui
                [&]() {
                    lerpCubemapShader->use();
                    float normalized = fmod(s_cycle.timeOfDay, 24.0f) / 24.0f;
                    lerpCubemapShader->setFloat("lerpFactor", std::clamp(normalized, 0.0f, 1.0f));

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, s_cycle.nightHDR.second.envCubemap);
                    lerpCubemapShader->setInt("prevCube", 0);

                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, s_cycle.dayHDR.second.envCubemap);
                    lerpCubemapShader->setInt("nextCube", 1);

                    for (int mip = 0; mip < 1; ++mip) {
                        lerpCubemapShader->setInt("mipLevel", mip);
                        glBindImageTexture(2, texBack, mip, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
                        int groups = (std::max(1, size >> mip) + 7) / 8;
                        glDispatchCompute(groups, groups, 6);
                    }

                    // limpeza das bindings usadas no dispatch
                    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    glBindImageTexture(2, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

                    // --- Cria fence ---
                    GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
                    glFlush();

                    ComputeSyncComponent::Enqueue(fence, [&]() {
                        // Barrier garante visibilidade da textura antes do swap
                        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

                        std::swap(texFront, texBack);
                        std::swap(texBack, texIdle);

                        // DEBUG opcional
                        /*std::cout << "[Cubemap] Swap atômico feito. front=" << texFront
                            << " back=" << texBack << " idle=" << texIdle << "\n";*/
                        });
                        jobActive = false;
                },
                // onComplete - **captura por referência** para poder fazer swap nas textures reais
                [&]() { 
                }
            );
        }

        return texFront;
    }

    // Retorna apenas o front seguro para render
    static GLuint GetSkyboxFront() { return texFront; }




    static GLuint CreateTestCubemap(int size) {
        // Flags de controle
        static bool jobActive = false;

        // Triple-buffering
        static GLuint texFront = 0;
        static GLuint texBack = 0;
        static GLuint texIdle = 0;

        auto initTexture = [&](GLuint& tex) {
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
        };

        // Inicializa texturas
        initTexture(texFront);
        initTexture(texBack);
        initTexture(texIdle);

        if (!jobActive) {
            jobActive = true;

            ComputeSyncComponent::Request(
                [=]() {
                    // Dispatch compute no texBack
                    computeShader->use();
                    computeShader->setFloat("uTime", (float)glfwGetTime());

                    glBindImageTexture(0, texBack, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
                    int groups = (size + 7) / 8;
                    glDispatchCompute(groups, groups, 6);

                    // Garante coerência de escrita
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                        GL_TEXTURE_FETCH_BARRIER_BIT);

                    glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
                },
                []() {
                    // Compute terminou → swap seguro
                    std::swap(texFront, texBack);
                    std::swap(texBack, texIdle);

                    jobActive = false;
                }
            );
        }

        // Sempre retorna o front (último pronto)
        return texFront;
    }


    static GLuint CreateTestCubemap2(int size) {
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

            computeShader->use();
            computeShader->setFloat("uTime", (float)glfwGetTime()); // precisa de uniform no shader
            glBindImageTexture(0, tex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

            int groups = (size + 7) / 8;
            glDispatchCompute(groups, groups, 6); // 6 faces
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

        }
        return tex;
    }

};













/*
// Dispara compute shader apenas se não houver job ativo
if (!jobActive) {
    jobActive = true;

    ComputeSyncComponent::Request(
        [size]() {
            // 1. Dispatch compute shader para texBack
            computeShader->use();
            computeShader->setFloat("uTime", (float)glfwGetTime());

            glBindImageTexture(0, texBack, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            int groups = (size + 7) / 8;
            glDispatchCompute(groups, groups, 6);

            // Barrier garante que texBack está pronta para leitura futura
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
            glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);


            // 4. Enfileira callback que só será chamado quando a GPU finalizar
            ComputeSyncComponent::Request([size]() {

                // 2. Enfileira copy GPU-side texBack → texIdle
                glCopyImageSubData(texBack, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                    texIdle, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                    size, size, 6);

                // 3. Cria fence para saber quando a GPU terminou
                GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

                // Swap seguro após compute + copy
                std::swap(texFront, texIdle);
                //std::swap(texBack, texIdle); // mantém triple-buffering
                jobActive = false;
                });
        },
        nullptr // callback principal não usado, fica no fence
    );
}*/








//ComputeSyncComponent::Request(
//    [size]() {
//        // Dispatch do compute shader na textura back
//        computeShader->use();
//        computeShader->setFloat("uTime", (float)glfwGetTime());
//
//        glBindImageTexture(0, texBack, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
//        int groups = (size + 7) / 8;
//        glDispatchCompute(groups, groups, 6);
//
//        // Barrier garante que texBack está pronta para leitura futura
//        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
//
//        glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
//    },
//    []() {
//        // Callback: só swap quando o compute terminou
//        jobActive = false;
//
//        // Swap seguro: front = render pronta, back = próxima para compute
//        //std::swap(texFront, texBack);
//        //std::swap(texBack, texIdle);
//    }
//);





//// Cria uma fence GPU para saber quando terminou
//GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
//
//// Depois, você pode esperar de forma não-bloqueante ou bloqueante:
//GLenum wait = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000); // timeout 1s
//if (wait == GL_ALREADY_SIGNALED || wait == GL_CONDITION_SATISFIED) {
//    // Compute terminou de fato, agora podemos copiar
//    glCopyImageSubData(
//        texTemp, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
//        texSkybox, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
//        size, size, 6
//    );
//}
//glDeleteSync(fence);








//        // --- Em cada frame, verificar se terminou ---
//if (job.active && job.fence) {
//    GLenum wait = glClientWaitSync(job.fence, 0, 0); // não bloqueante
//    if (wait == GL_ALREADY_SIGNALED || wait == GL_CONDITION_SATISFIED) {
//        // Compute terminou, agora é seguro copiar
//        glCopyImageSubData(
//            job.texBack, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
//            job.texFront, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
//            job.size, job.size, 6
//        );
//
//        // Callback seguro: swap ou outro processamento
//        job.active = false;
//        if (job.fence) {
//            glDeleteSync(job.fence);
//            job.fence = 0;
//        }
//        // Exemplo de swap
//        std::swap(job.texFront, job.texBack);
//    }
//}


                //comecando com uma cor
//for (int face = 0; face < 6; ++face) {
//    std::vector<float> black(size * size * 4, 0.0f); // RGBA preto
//    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
//        GL_RGBA16F, size, size, 0,
//        GL_RGBA, GL_FLOAT, black.data());
//}