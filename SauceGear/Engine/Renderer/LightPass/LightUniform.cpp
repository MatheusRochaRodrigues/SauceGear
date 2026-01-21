#include "LightUniform.h" 
#include "LightPass.h" 
#include "LightMakers/CascadeSun.h" 
#include "ShadowPool.h" 
#include "../../Core/EngineContext.h" 
#include "../../Scene/SceneECS.h" 
#include "../../Core/Camera.h" 
#include "../../Graphics/Shader.h"
#include <glm/glm.hpp>   

namespace UniformLights {
    //------------------------------------ utilidades para enviar dados aos shaders (mantive a interface) 
    int SetSunToShader(Shader* shader) {
        GLuint currentSun = LightPass::currentSun;
        if (currentSun == INVALID_ENTITY) return 0;
        shader->use();
        auto& light = GEngine->scene->GetComponent<LightComponent>(currentSun);
        auto& transform = GEngine->scene->GetComponent<TransformComponent>(currentSun);

        shader->setFloat("farPlane", GEngine->mainCamera->farClip);

        std::string prefix = "light";
        shader->setVec3(prefix + ".color", light.color);
        shader->setVec3(prefix + ".direction", transform.GetForwardDirection());
        shader->setFloat(prefix + ".intensity", light.intensity);
        shader->setMat4("view", GEngine->mainCamera->GetViewMatrix()); // view da câmera

        // usa CascadeSun diretamente
        const auto& cascadeDistances = CascadeSun::GetCascadeDistances();
        shader->setFloatArray("cascadePlaneDistances", cascadeDistances.data(), static_cast<int>(cascadeDistances.size())); // Envia o array inteiro de uma vez
        shader->setInt("cascadeCount", (int)cascadeDistances.size());

        // vincula a texture array do shadow map
        shader->setInt("cascadeShadowMap", 7);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D_ARRAY, CascadeSun::cascadeDepthMapArray);

        return 1;
    }


    void set_uShadowData(Shader* lightingShader, int unit) {
        //shadows
        lightingShader->setInt("cascadeShadowMap", unit); // deve bater com GL_TEXTURE4
        // --- Shadow map array (sampler2DArray)
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D_ARRAY, CascadeSun::cascadeDepthMapArray);

        // --- Cascade count
        lightingShader->setInt("cascadeCount", CascadeSun::GetCascadeCount());

        // --- Cascade distances
        auto& distances = CascadeSun::GetCascadeDistances();
        lightingShader->setFloatArray("cascadePlaneDistances", distances.data(), (int)distances.size());

        // --- View matrix (para converter world→view)
        lightingShader->setMat4("view", GEngine->mainCamera->GetViewMatrix());
    }

    void SetPointsToShader(Shader* shader, int i) {
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

    void SetLightsToShader(Shader* shader) {
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
}