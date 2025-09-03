#include "LightSystem.h"

LightSystem::LightSystem() {
    glGenFramebuffers(1, &depthMapFBO);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    // are clearly defining here that we do not want to draw any color information
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    // returning to the default frame buffer state
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    currentSun = INVALID_ENTITY;

    // Carregar dados das luzes em SSBO
    glGenBuffers(1, &lightSSBO);
    glBindBuffer(GL_UNIFORM_BUFFER, lightSSBO);
    offBuff = 144;
    glBufferData(GL_UNIFORM_BUFFER, MAX_LIGHTS * offBuff, NULL, GL_STATIC_DRAW); // allocate 160 bytes of memory 
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightSSBO);

    //// Carregar mapas de sombra em um segundo SSBO
    //glGenBuffers(1, &shadowMapSSBO);
    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, shadowMapSSBO);
    //glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_LIGHTS * 192, NULL, GL_STATIC_DRAW); // allocate 192 bytes of memory 
    //glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shadowMapSSBO);


    poolShadowsTex.push_back(ShadowSettings{ ShadowLOD::HIGH,    std::deque<GLuint>(), std::deque<GLuint>() });  //1024
    poolShadowsTex.push_back(ShadowSettings{ ShadowLOD::MEDIUM,  std::deque<GLuint>(), std::deque<GLuint>() });  //512
    poolShadowsTex.push_back(ShadowSettings{ ShadowLOD::LOW,     std::deque<GLuint>(), std::deque<GLuint>() });  //256  

     
    //InitCascade();
}

void LightSystem::InitCascade() {
    // Cascade Shadow Setup
    shadowCascadeLevels = {
        GEngine->mainCamera->farClip / 50.0f,
        GEngine->mainCamera->farClip / 25.0f,
        GEngine->mainCamera->farClip / 10.0f,
        GEngine->mainCamera->farClip / 2.0f
    };

    glGenFramebuffers(1, &cascadeFBO);
    glGenTextures(1, &cascadeDepthMapArray);

    int cascadeResolution = 2048;
    glBindTexture(GL_TEXTURE_2D_ARRAY, cascadeDepthMapArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
        cascadeResolution, cascadeResolution, MAX_CASCADES,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, cascadeFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cascadeDepthMapArray, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    //UBO
    // UBO para cascaded shadows
    GLuint lightMatricesUBO;
    glGenBuffers(1, &lightMatricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, lightMatricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 16, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Vincula o UBO ao binding point 0 (pode escolher outro se quiser)
    /*GLuint blockIndex = glGetUniformBlockIndex(GEngine->renderer->GetShadowShader_Sun->ID, "LightSpaceMatrices");
    glUniformBlockBinding(GEngine->renderer->GetShadowShader_Sun->ID, blockIndex, 2);*/

    // Liga o buffer ao binding point 0
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightMatricesUBO);

    // Guarda em membro estático (pra não recriar sempre)
    cascadeMatricesUBO = lightMatricesUBO;

}

static std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& projview) {
    glm::mat4 inv = glm::inverse(projview);
    std::vector<glm::vec4> corners;
    for (int x = 0; x < 2; x++) for (int y = 0; y < 2; y++) for (int z = 0; z < 2; z++) {
        glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
        corners.push_back(pt / pt.w);
    }
    return corners;
}

std::vector<glm::mat4> LightSystem::GetLightSpaceMatrices(const glm::vec3& lightDir) {
    auto camera = GEngine->mainCamera;

    std::vector<glm::mat4> matrices;
    float nearClip = camera->nearClip;
    float farClip = camera->farClip;

    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 proj = glm::perspective(glm::radians(camera->Zoom),   //fov
        camera->aspectRatio,
        nearClip, farClip);

    for (size_t i = 0; i < shadowCascadeLevels.size(); i++) {
        float splitNear = (i == 0) ? nearClip : shadowCascadeLevels[i - 1];
        float splitFar = shadowCascadeLevels[i];

        glm::mat4 projSplit = glm::perspective(glm::radians(camera->Zoom),
            camera->aspectRatio,
            splitNear, splitFar);

        // Pega os cantos do frustum
        std::vector<glm::vec4> frustumCorners = GetFrustumCornersWorldSpace(projSplit * view);

        glm::vec3 center(0.0f);
        for (auto& v : frustumCorners) center += glm::vec3(v);
        center /= frustumCorners.size();

        glm::mat4 lightView = glm::lookAt(center - lightDir * 50.0f, center, glm::vec3(0, 1, 0));

        // Define limites ortográficos
        float minX = FLT_MAX, maxX = -FLT_MAX;
        float minY = FLT_MAX, maxY = -FLT_MAX;
        float minZ = FLT_MAX, maxZ = -FLT_MAX;
        for (auto& corner : frustumCorners) {
            glm::vec4 trf = lightView * corner;
            minX = std::min(minX, trf.x);
            maxX = std::max(maxX, trf.x);
            minY = std::min(minY, trf.y);
            maxY = std::max(maxY, trf.y);
            minZ = std::min(minZ, trf.z);
            maxZ = std::max(maxZ, trf.z);
        }

        glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ - 10.0f, maxZ + 10.0f);
        matrices.push_back(lightProjection * lightView);
    }
    return matrices;
}


void LightSystem::Sun() {
    // Se não houver um "sol" atual, atribui o primeiro light directional como sol
    // Verifica se o "sol" atual foi deletado ou removido
    if (currentSun == INVALID_ENTITY) {           //if (currentSun != INVALID_ENTITY) return;
        //if (currentSun != nullptr && GEngine->scene->EntityExists(currentSun)) return; 
        auto entities = GEngine->scene->GetEntitiesWith<LightComponent, Transform>();
        // Se o "sol" for removido, troque pelo próximo directional ativo
        for (auto& lightEntity : entities) {
            auto& light = GEngine->scene->GetComponent<LightComponent>(lightEntity);
            if (light.type == ShadowType::Directional) {
                currentSun = lightEntity;
                break;
            }
        }
    }
    if (currentSun == INVALID_ENTITY) return;

    auto& transform = GEngine->scene->GetComponent<Transform>(currentSun);
    auto& l = GEngine->scene->GetComponent<LightComponent>(currentSun);
    if (l.depthMap == 0) l.depthMap = GetAvailableShadowMap(ShadowType::Directional, ShadowLOD::HIGH);

    UpdateDirectional(l, transform.rotation, GetSettingsForLOD(ShadowLOD::HIGH), l.depthMap); 

    // Gera cascades
    lightSpaceMatrices = GetLightSpaceMatrices(transform.GetForwardDirection());

    Shader* shadowShader = GEngine->renderer->GetShadowShader_Sun;
    shadowShader->use();

    // Atualiza com as matrizes
    /*for (size_t i = 0; i < lightSpaceMatrices.size(); i++) {
        shadowShader->setMat4("lightSpaceMatrices[" + std::to_string(i) + "]", lightSpaceMatrices[i]);
    }*/ 

    // Atualiza UBO com as matrizes
    glBindBuffer(GL_UNIFORM_BUFFER, cascadeMatricesUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * lightSpaceMatrices.size(), lightSpaceMatrices.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //

    glViewport(0, 0, 2048, 2048);
    glBindFramebuffer(GL_FRAMEBUFFER, cascadeFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    GEngine->renderer->RenderSceneWithShader(shadowShader);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

ShadowLOD LightSystem::ComputeLOD(float distance) {
    if (distance < 25.0f) return ShadowLOD::HIGH;
    if (distance < 40.0f) return ShadowLOD::MEDIUM;
    if (distance < 60.0f) return ShadowLOD::LOW;
    return ShadowLOD::NONE;
}

unsigned int LightSystem::GetSettingsForLOD(ShadowLOD lod) {
    switch (lod) {
    case ShadowLOD::HIGH:   return 1024;  // nítido
    case ShadowLOD::MEDIUM: return 512;  // leve desfoque
    case ShadowLOD::LOW:    return 256;  // borrado 
    }
    return 0; // fallback
}

void LightSystem::UpdateDirectional(LightComponent& light, const glm::vec3& pos, unsigned int resolution, GLuint texture) { 
    Shader* shadowShader = GEngine->renderer->GetShadowShader_Directional;
    glm::mat4 lightProjection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 1.f, 7.5f);            //light.range 
    glm::mat4 lightView = glm::lookAt(pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    light.lightSpaceMatrix = lightProjection * lightView;
    light.position = pos;

    shadowShader->use();
    shadowShader->setMat4("lightSpaceMatrix", light.lightSpaceMatrix);

    glViewport(0, 0, resolution, resolution);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);      //light.depthMap
    // are clearly defining here that we do not want to draw any color information
    //glDrawBuffer(GL_NONE);
    //glReadBuffer(GL_NONE);
    verify();
    glClear(GL_DEPTH_BUFFER_BIT);

    GEngine->renderer->RenderSceneWithShader(shadowShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GEngine->window->SetWindowViewport0();
}

void LightSystem::UpdatePoint(LightComponent& light, const glm::vec3& pos, unsigned int resolution, GLuint texture) { 
    Shader* pointShadow = GEngine->renderer->GetShadowShader_Point;
    float nearPlane = 0.1f, farPlane = light.range;

    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);
    glm::vec3 dirs[6] = {
        {1,0,0}, {-1,0,0}, {0,1,0},
        {0,-1,0}, {0,0,1}, {0,0,-1}
    };
    glm::vec3 ups[6] = {
        {0,-1,0}, {0,-1,0}, {0,0,1},
        {0,0,-1}, {0,-1,0}, {0,-1,0}
    };

    glm::mat4 shadowTransforms[6] = {};
    for (int i = 0; i < 6; i++)
        shadowTransforms[i] = shadowProj * glm::lookAt(pos, pos + dirs[i], ups[i]);

    pointShadow->use();
    for (int i = 0; i < 6; ++i)
        pointShadow->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
    pointShadow->setVec3("lightPos", pos);
    pointShadow->setFloat("far_plane", farPlane);

    glViewport(0, 0, resolution/2, resolution/2);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
    verify();
    glClear(GL_DEPTH_BUFFER_BIT);

    GEngine->renderer->RenderSceneWithShader(pointShadow);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GEngine->window->SetWindowViewport0();
}

void LightSystem::verify() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer incompleto: " << status << std::endl;
    }
}


std::vector<Entity> LightSystem::SelectLightsToCastShadow(const std::vector<Entity>& lights) { 
    std::vector<Entity> selectedEntities;

    int Directional = 0;
    int Point = 0;
    for (const auto& light : lights) {
        const auto& lc = GEngine->scene->GetComponent<LightComponent>(light);
        if (!lc.castShadow) continue;

        switch (lc.type) {
        case ShadowType::Directional:
            if (Directional >= MAX_LIGHTS) break;                       // if (selectedDirectional.size() >= MAX_LIGHTS) break;
            if (lc.castShadow)
                selectedEntities.push_back(light);                        // selectedDirectional.push_back(light);
            Directional++;
            break;
        case ShadowType::Point:
            if (Point >= MAX_LIGHTS) break;
            if (lc.castShadow)
                selectedEntities.push_back(light);
            Point++;
            break;
        }
    }
    return selectedEntities;
};


std::vector<Entity> LightSystem::GetClosestLights(const glm::vec3& playerPos) {
    auto all = GEngine->scene->GetEntitiesWith<LightComponent, Transform>(); 

    std::vector<std::pair<float, Entity>> sorted;
    sorted.reserve(all.size());

    for (Entity e : all) {
        const auto& transform = GEngine->scene->GetComponent<Transform>(e);
        float dist = glm::distance(playerPos, transform.position);
        sorted.emplace_back(dist, e);
    }

    std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) {
        return a.first < b.first;
        });

    std::vector<Entity> result;
    for (int i = 0; i < std::min(MAX_LIGHTS, (int)sorted.size()); ++i) {
        result.push_back(sorted[i].second);
    }

    return result;
}

LightGroups LightSystem::GroupLightsByType(const std::vector<Entity>& lights) {
    LightGroups groups;
    for (Entity e : lights) {
        const auto& light = GEngine->scene->GetComponent<LightComponent>(e);
        switch (light.type) {
        case ShadowType::Directional: groups.directional.push_back(e); break;
        case ShadowType::Point:       groups.point.push_back(e);       break;
        case ShadowType::Spot:        groups.spot.push_back(e);        break;
        }
    }
    return groups;
}
 
void LightSystem::ReturnShadowMapToPool(ShadowLOD lod, GLuint shadowMap, ShadowType type) {
    ShadowSettings& settings = poolShadowsTex[static_cast<int>(lod)];  // Ajuste para o LOD correto
    if (type == ShadowType::Directional) {
        settings.availableDirectionalMaps.push_back(shadowMap);
    }
    else if (type == ShadowType::Point) {
        settings.availablePointMaps.push_back(shadowMap);
    }
}

GLuint LightSystem::GetAvailableShadowMap(ShadowType type, ShadowLOD lod) { 
    ShadowSettings& settings = poolShadowsTex[static_cast<int>(lod)];  // Ajuste para o LOD correto
    GLuint shadowMap = 0;

    int resolution = GetSettingsForLOD(lod);

    if (type == ShadowType::Directional) {
        if (!settings.availableDirectionalMaps.empty()) {
            shadowMap = settings.availableDirectionalMaps.front();
            settings.availableDirectionalMaps.pop_front();
        }
        else {
            glGenTextures(1, &shadowMap);
            glBindTexture(GL_TEXTURE_2D, shadowMap);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); 

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);       //GL_NEAREST
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        }
    }
    else if (type == ShadowType::Point) {
        if (!settings.availablePointMaps.empty()) {
            shadowMap = settings.availablePointMaps.front();
            settings.availablePointMaps.pop_front();
        }
        else {
            resolution = resolution / 2;    //4

            glGenTextures(1, &shadowMap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap);
            for (unsigned int i = 0; i < 6; ++i) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        }
    }

    return shadowMap;
}



//ShadowSettings* LightSystem::GetShadowSettings(ShadowLOD lod) {
//    for (auto& [key, settings] : poolShadowsTex) {
//        if (key == lod)
//            return &settings;
//    }
//    return nullptr;
//}