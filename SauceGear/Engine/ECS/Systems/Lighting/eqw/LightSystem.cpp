#include "LightSystem.h"

// inicialização mínima — a maioria dos recursos está no pool/cascade
LightSystem::LightSystem() {
    std::cout << "[LightSystem] ctor\n";

    // cria SSBO para luzes (como antes)
    glGenBuffers(1, &lightSSBO);
    glBindBuffer(GL_UNIFORM_BUFFER, lightSSBO);
    offBuff = 144;
    glBufferData(GL_UNIFORM_BUFFER, MAX_LIGHTS * offBuff, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightSSBO);

    // depthMapFBO será criado/gerenciado em ShadowPool::Init()
    ShadowPool::Init();      // garante FBO/textures pool
    CascadeSun::Init();          // inicializa cascades do sol
}

void LightSystem::Init() {
    // caso queira alguma inicialização adicional
}

void LightSystem::verify() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer incompleto: " << status << std::endl;
    }
}

void LightSystem::Update(float dt) {
    glm::vec3 posPlayer = GEngine->mainCamera->Position;

    // obter as luzes mais próximas que podem lançar sombras
    auto closest = GetClosestLights(posPlayer);
    auto selected = SelectLightsToCastShadow(closest);
    LightGroups groups = GroupLightsByType(selected);

    // Limpa os arrays que vamos preencher
    lightInActive.point.clear();
    lightInActive.directional.clear();
    lightInActive.spot.clear();

    // Atualiza o sol (cascades) se existir
    if (currentSun != INVALID_ENTITY) {
        // pegamos componente do sol
        auto& transform = GEngine->scene->GetComponent<Transform>(currentSun);
        auto& light = GEngine->scene->GetComponent<LightComponent>(currentSun);

        if (light.castShadow && light.type == LightType::Directional) {
            CascadeSun::UpdateSunShadow(light, transform);
            lightInActive.directional.push_back(currentSun);


            // Atualiza shadowCascadeLevels local para shaders
            shadowCascadeLevels = CascadeSun::GetCascadeDistances();

            // garante que ShadowMaps possui a entrada do sol (opcional)
            /*if (ShadowPool::ShadowMaps.find(currentSun) == ShadowPool::ShadowMaps.end()) {
                ShadowPool::ShadowMaps[currentSun] = { ShadowLOD::HIGH, -1 };
            }*/
        }
    }
    else {
        // tenta encontrar um directional para ser sun (preserva comportamento anterior)
        auto entities = GEngine->scene->GetEntitiesWith<LightComponent, Transform>();
        for (auto& ent : entities) {
            auto& lc = GEngine->scene->GetComponent<LightComponent>(ent);
            if (lc.type == LightType::Directional) {
                currentSun = ent;
                break;
            }
        }
    }

    // Para cada luz selecionada (exceto o sol tratado acima) atualiza shadowmaps
    for (auto& e : selected) {
        if (e == currentSun) continue;
        auto& transform = GEngine->scene->GetComponent<Transform>(e);
        auto& l = GEngine->scene->GetComponent<LightComponent>(e);

        ShadowLOD lod = ComputeLOD(glm::distance(transform.position, posPlayer));
        if (lod == ShadowLOD::NONE) continue;

        if (l.depthMap == 0) {
            l.depthMap = ShadowPool::GetAvailableShadowMap(l.type, lod);
            ShadowPool::ShadowMaps[e] = { lod, -1 };
        }

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        if (l.type == LightType::Point) {
            LightPoint::UpdatePoint(l, transform.position, ShadowPool::GetSettingsForLOD(lod), ShadowPool::depthMapFBO, l.depthMap);
            lightInActive.point.push_back(e);
        }
        else if (l.type == LightType::Directional) {
            LightDirectional::UpdateDirectional(l, transform.position, ShadowPool::GetSettingsForLOD(lod), ShadowPool::depthMapFBO, l.depthMap);
            lightInActive.directional.push_back(e);
        }

        glCullFace(GL_BACK);
    }

    // devolve sombras que saíram do alcance
    HandleShadowMapReturn(posPlayer);

    // finalmente escrever dados para SSBO
    SetLightsToSSBO();
}

void LightSystem::HandleShadowMapReturn(const glm::vec3& playerPosition) {
    std::vector<Entity> toErase;
    for (auto& [lightEntity, lodMapShadow] : ShadowPool::ShadowMaps) {
        // segurança: se a entidade foi removida, devolve
        if (!GEngine->scene->EntityExists(lightEntity)) {
            auto it = ShadowPool::ShadowMaps.find(lightEntity);
            if (it != ShadowPool::ShadowMaps.end()) {
                auto& maybeLight = it->first;
                auto& pair = it->second;
                // aqui não temos depthMap diretamente -> pega do componente se existir
                if (GEngine->scene->HasComponent<LightComponent>(lightEntity)) {
                    auto& comp = GEngine->scene->GetComponent<LightComponent>(lightEntity);
                    if (comp.depthMap != 0) {
                        ShadowPool::ReturnShadowMapToPool(pair.first, comp.depthMap, comp.type);
                        comp.depthMap = 0;
                    }
                }
                toErase.push_back(lightEntity);
            }
            continue;
        }

        auto& transform = GEngine->scene->GetComponent<Transform>(lightEntity);
        auto& light = GEngine->scene->GetComponent<LightComponent>(lightEntity);

        ShadowLOD currentLod = ComputeLOD(glm::distance(transform.position, playerPosition));
        // se LOD mudou para NONE ou diferente da armazenada => devolver
        if (ShadowPool::ShadowMaps.find(lightEntity) != ShadowPool::ShadowMaps.end()) {
            auto stored = ShadowPool::ShadowMaps[lightEntity];
            if (currentLod != stored.first) {
                // devolve
                if (light.depthMap != 0) {
                    ShadowPool::ReturnShadowMapToPool(stored.first, light.depthMap, light.type);
                    light.depthMap = 0;
                }
                toErase.push_back(lightEntity);
            }
        }
    }

    for (auto& e : toErase) ShadowPool::ShadowMaps.erase(e);
}

std::vector<Entity> LightSystem::SelectLightsToCastShadow(const std::vector<Entity>& lights) {
    std::vector<Entity> selectedEntities;
    int Directional = 0;
    int Point = 0;
    for (const auto& light : lights) {
        const auto& lc = GEngine->scene->GetComponent<LightComponent>(light);
        if (!lc.castShadow) continue;
        switch (lc.type) {
        case LightType::Directional:
            if (Directional >= MAX_LIGHTS) break;
            selectedEntities.push_back(light);
            Directional++;
            break;
        case LightType::Point:
            if (Point >= MAX_LIGHTS) break;
            selectedEntities.push_back(light);
            Point++;
            break;
        default:
            break;
        }
    }
    return selectedEntities;
}

LightGroups LightSystem::GroupLightsByType(const std::vector<Entity>& lights) {
    LightGroups groups;
    for (Entity e : lights) {
        const auto& light = GEngine->scene->GetComponent<LightComponent>(e);
        switch (light.type) {
        case LightType::Directional: groups.directional.push_back(e); break;
        case LightType::Point:       groups.point.push_back(e);       break;
        case LightType::Spot:        groups.spot.push_back(e);        break;
        }
    }
    return groups;
}

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

int LightSystem::SetSunToShader(Shader* shader) {
    if (currentSun == INVALID_ENTITY) return 0;

    shader->use();
    auto& light = GEngine->scene->GetComponent<LightComponent>(currentSun);
    auto& transform = GEngine->scene->GetComponent<Transform>(currentSun);

    std::string prefix = "light";
    shader->setVec3(prefix + ".position", transform.position);
    shader->setVec3(prefix + ".color", light.color);
    shader->setInt(prefix + ".type", static_cast<int>(light.type));
    shader->setInt(prefix + ".castS", 1);
    shader->setVec3(prefix + ".direction", transform.GetForwardDirection());
    shader->setMat4(prefix + ".lightMatrix", light.lightSpaceMatrix);

    // view da câmera
    shader->setMat4("view", GEngine->mainCamera->GetViewMatrix());

    // usa CascadeSun diretamente
    const auto& cascadeDistances = CascadeSun::GetCascadeDistances();
    for (size_t i = 0; i < cascadeDistances.size(); ++i) {
        shader->setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", cascadeDistances[i]);
    }
    shader->setInt("cascadeCount", (int)cascadeDistances.size());

    // vincula a texture array do shadow map
    shader->setInt("cascadeShadowMap", 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D_ARRAY, CascadeSun::cascadeDepthMapArray);

    return 1;
}


void LightSystem::SetPointsToShader(Shader* shader, int i) {
    shader->use();
    for (auto& [lightEntity, map] : ShadowPool::ShadowMaps) {
        int lightCount = map.second + i;
        auto& light = GEngine->scene->GetComponent<LightComponent>(lightEntity);
        if (light.type == LightType::Point) {
            shader->setInt("pointShadows[" + std::to_string(lightCount) + "]", lightCount);
            glActiveTexture(GL_TEXTURE0 + lightCount);
            glBindTexture(GL_TEXTURE_CUBE_MAP, light.depthMap);
        }
    }
    shader->setFloat("far_plane", 25.0f);
    shader->setInt("numLights", (int)ShadowPool::ShadowMaps.size());
    shader->setVec3("viewPos", GEngine->mainCamera->Position);
}

void LightSystem::SetLightsToShader(Shader* shader) {
    shader->use();
    for (auto& [lightEntity, map] : ShadowPool::ShadowMaps) {
        int lightCount = map.second;
        auto& light = GEngine->scene->GetComponent<LightComponent>(lightEntity);
        if (light.type == LightType::Point) {
            shader->setInt("pointShadows[" + std::to_string(lightCount) + "]", lightCount);
            glActiveTexture(GL_TEXTURE0 + lightCount);
            glBindTexture(GL_TEXTURE_CUBE_MAP, light.depthMap);
        }
        else {
            shader->setInt("shadowMaps[" + std::to_string(lightCount) + "]", lightCount);
            glActiveTexture(GL_TEXTURE0 + lightCount);
            glBindTexture(GL_TEXTURE_2D, light.depthMap);
        }
    }

    shader->setFloat("far_plane", 25.0f);
    shader->setInt("numLights", (int)ShadowPool::ShadowMaps.size());
    shader->setInt("albedoMap", 0);
    shader->setVec3("viewPos", GEngine->mainCamera->Position);
}

void LightSystem::SetLightsToSSBO() {
    glBindBuffer(GL_UNIFORM_BUFFER, lightSSBO);
    int shadowMapIndexPoint = 0;
    int shadowMapIndexDir = 0;
    int i = 0;

    ForEachLight(lightInActive, [&](Entity& lightEntity) {
        auto& light = GEngine->scene->GetComponent<LightComponent>(lightEntity);
        auto& transform = GEngine->scene->GetComponent<Transform>(lightEntity);

        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 0, sizeof(int), &light.type);
        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 16, sizeof(glm::vec3), glm::value_ptr(transform.position));
        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 32, sizeof(glm::vec3), &light.color);
        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 48, sizeof(float), &light.intensity);
        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 52, sizeof(float), &light.range);
        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 56, sizeof(float), &light.angle);
        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 60, sizeof(bool), &light.castShadow);
        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 64, sizeof(glm::mat4), glm::value_ptr(light.lightSpaceMatrix));

        int shadowMapIdx = -1;
        if (light.depthMap != 0) {
            if (light.type == LightType::Directional) {
                shadowMapIdx = shadowMapIndexDir++;
                ShadowPool::ShadowMaps[lightEntity].second = shadowMapIdx;
            }
            else {
                shadowMapIdx = shadowMapIndexPoint++;
                ShadowPool::ShadowMaps[lightEntity].second = shadowMapIdx;
            }
        }

        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 128, sizeof(GLuint), &shadowMapIdx);
        i++;
        });

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

ShadowLOD LightSystem::ComputeLOD(float distance) {
    if (distance < 25.0f) return ShadowLOD::HIGH;
    if (distance < 40.0f) return ShadowLOD::MEDIUM;
    if (distance < 60.0f) return ShadowLOD::LOW;
    return ShadowLOD::NONE;
}

unsigned int LightSystem::GetSettingsForLOD(ShadowLOD lod) {
    return ShadowPool::GetSettingsForLOD(lod);
}
