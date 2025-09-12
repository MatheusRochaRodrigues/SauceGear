/*
static inline GLuint texFront = 0;
static void CreateTestCubemap3(int size) {
    static bool jobActive = false;
    static GLuint texBack = 0;
    static GLuint texIdle = 0;

    // -------------------------------------------------
    auto initTexture = [&](GLuint& tex, glm::vec4 color) {
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

            // Preenche com cor sólida pra debug
            std::vector<glm::vec4> data(size * size, color);
            for (int face = 0; face < 6; ++face) {
                glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
                    0, 0, size, size, GL_RGBA, GL_FLOAT, data.data());
            }
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }
        };
    // -------------------------------------------------

    initTexture(texFront, glm::vec4(1, 0, 0, 1)); // vermelho
    initTexture(texBack, glm::vec4(0, 1, 0, 1)); // verde
    initTexture(texIdle, glm::vec4(0, 0, 1, 1)); // azul

    if (jobActive) return;
    jobActive = true;

    ComputeSyncComponent::Request(
        [=]() { // gpuWork
            lerpCubemapShader->use();
            float normalized = fmod(s_cycle.timeOfDay, 24.0f) / 24.0f;
            lerpCubemapShader->setFloat("lerpFactor", std::clamp(normalized, 0.0f, 1.0f));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, s_cycle.nightHDR.second.envCubemap);
            lerpCubemapShader->setInt("prevCube", 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, s_cycle.dayHDR.second.envCubemap);
            lerpCubemapShader->setInt("nextCube", 1);

            glBindImageTexture(2, texBack, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            int groups = (std::max(1, size) + 7) / 8;
            glDispatchCompute(groups, groups, 6);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        },
        []() { // onComplete
            // Swap seguro
            std::swap(texFront, texBack);
            std::swap(texBack, texIdle);
            jobActive = false;
        }
    );
}

// Retorna apenas o front seguro para render
static GLuint GetSkyboxFront() { return texFront; }

*/






/*

static GLuint CreateTestCubemapDebug(int size) {
    static bool jobActive = false;
    static GLuint texFront = 0;
    static GLuint texBack = 0;
    static GLuint texIdle = 0;

    // -------------------------------------------------
    auto initTexture = [&](GLuint& tex, glm::vec4 color) {
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

            // Preenche com cor sólida pra debug
            std::vector<glm::vec4> data(size * size, color);
            for (int face = 0; face < 6; ++face) {
                glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
                    0, 0, size, size, GL_RGBA, GL_FLOAT, data.data());
            }
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }
        };
    // -------------------------------------------------

    initTexture(texFront, glm::vec4(1, 0, 0, 1)); // vermelho
    initTexture(texBack, glm::vec4(0, 1, 0, 1)); // verde
    initTexture(texIdle, glm::vec4(0, 0, 1, 1)); // azul

    if (!jobActive) {
        jobActive = true;

        ComputeSyncComponent::Request(
            // gpuWork
            [&]() {
                lerpCubemapShader->use();
                float normalized = fmod(s_cycle.timeOfDay, 24.0f) / 24.0f;
                lerpCubemapShader->setFloat("lerpFactor",
                    std::clamp(normalized, 0.0f, 1.0f));

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, s_cycle.nightHDR.second.envCubemap);
                lerpCubemapShader->setInt("prevCube", 0);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_CUBE_MAP, s_cycle.dayHDR.second.envCubemap);
                lerpCubemapShader->setInt("nextCube", 1);

                for (int mip = 0; mip < 1; ++mip) {
                    lerpCubemapShader->setInt("mipLevel", mip);
                    glBindImageTexture(2, texBack, mip, GL_TRUE, 0,
                        GL_WRITE_ONLY, GL_RGBA16F);
                    int groups = (std::max(1, size >> mip) + 7) / 8;
                    glDispatchCompute(groups, groups, 6);
                }

                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                glBindImageTexture(2, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

                GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
                glFlush();

                ComputeSyncComponent::Enqueue(fence, [&]() {
                    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT |
                        GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

                    // ⚠️ Teste de sincronização forçada
                    bool forceFinish = false;
                    if (forceFinish) glFinish();

                    std::swap(texFront, texBack);
                    std::swap(texBack, texIdle);

                    std::cout << "[Swap] Front=" << texFront
                        << " Back=" << texBack
                        << " Idle=" << texIdle << std::endl;

                    jobActive = false;
                    });
            },
            // onComplete
            [&]() {}
        );
    }

    // 🔎 Log de uso no render
    std::cout << "[Skybox] Usando front=" << texFront << std::endl;

    return texFront;
}


*/





















//static GLuint CreateTestCubemap3(int size) {
//    // Flags
//    static bool jobActive = false;
//    static bool swapReady = false; // atraso de 1 frame
//
//    // Triple-buffering
//    static GLuint texFront = 0;
//    static GLuint texBack = 0;
//    static GLuint texIdle = 0;
//
//    auto initTexture = [&](GLuint& tex) {
//        if (tex == 0) {
//            glGenTextures(1, &tex);
//            glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
//            for (int face = 0; face < 6; ++face) {
//                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
//                    GL_RGBA16F, size, size, 0, GL_RGBA, GL_FLOAT, nullptr);
//            }
//            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
//        }
//        };
//
//    initTexture(texFront);
//    initTexture(texBack);
//    initTexture(texIdle);
//
//    // --- Swap atrasado: só aplica agora se compute do frame passado terminou ---
//    if (swapReady) {
//        std::swap(texFront, texBack);
//        std::swap(texBack, texIdle);
//        swapReady = false; // reset
//    }
//
//    // --- Inicia job novo se não tem outro em andamento ---
//    if (!jobActive) {
//        jobActive = true;
//
//        ComputeSyncComponent::Request(
//            // GPU work
//            [&]() {
//                lerpCubemapShader->use();
//                float normalized = fmod(s_cycle.timeOfDay, 24.0f) / 24.0f;
//                lerpCubemapShader->setFloat("lerpFactor", std::clamp(normalized, 0.0f, 1.0f));
//
//                glActiveTexture(GL_TEXTURE0);
//                glBindTexture(GL_TEXTURE_CUBE_MAP, s_cycle.nightHDR.second.envCubemap);
//                lerpCubemapShader->setInt("prevCube", 0);
//
//                glActiveTexture(GL_TEXTURE1);
//                glBindTexture(GL_TEXTURE_CUBE_MAP, s_cycle.dayHDR.second.envCubemap);
//                lerpCubemapShader->setInt("nextCube", 1);
//
//                for (int mip = 0; mip < 1; ++mip) {
//                    lerpCubemapShader->setInt("mipLevel", mip);
//                    glBindImageTexture(2, texBack, mip, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
//
//                    int groups = (std::max(1, size >> mip) + 7) / 8;
//                    glDispatchCompute(groups, groups, 6);
//                }
//
//                // limpeza
//                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
//                glBindImageTexture(2, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
//
//                // garante visibilidade das writes
//                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
//            },
//            // Callback: marca swap para o PRÓXIMO frame
//            [&]() {
//                swapReady = true;
//                jobActive = false;
//            }
//        );
//    }
//
//    // Sempre retorna o último front válido
//    return texFront;
//}