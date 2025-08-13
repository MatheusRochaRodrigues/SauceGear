#pragma once 
#include "../System.h" 
#include "../../Core/EngineContext.h"
#include "../../Core/Time.h"
#include "../../Graphics/Renderer.h"
#include "../../Scene/SceneECS.h"
#include "../../Scene/Components/ComponentsHelper.h"

#define MAX_LIGHTS_PROJECTION 3


struct ShadowSettings {
    ShadowLOD resolution;                                             //int resolution;
    std::deque<GLuint> availableDirectionalMaps;
    std::deque<GLuint> availablePointMaps;
};

struct LightGroups {
    std::vector<Entity> directional;
    std::vector<Entity> point;
    std::vector<Entity> spot;
};

class LightSystem : public System {
public:
    LightSystem();

    void Update(float dt) override {
        glm::vec3 posPlayer = GEngine->mainCamera->Position;
        LightGroups closestGroupLights = GroupLightsByType(SelectLightsToCastShadow(GetClosestLights(posPlayer)));

        HandleSunChange();
        HandleShadowMapReturn(posPlayer);
        lightInActive.clear();
        auto renderLight = [&](Entity light) {
            if (light == currentSun) return;

            auto& transform = GEngine->scene->GetComponent<Transform>(light);
            auto& l = GEngine->scene->GetComponent<LightComponent>(light); 
             
            // Aqui você usa o LOD para escolher qual ShadowSettings usar 
            ShadowLOD lod = ComputeLOD(glm::distance(transform.position, posPlayer));
            if (lod == ShadowLOD::NONE) return;

            lightInActive.push_back(light);
            
            if (l.depthMap == 0) { // Utilize GetAvailableShadowMap para obter o shadowMap adequado  
                l.depthMap = GetAvailableShadowMap(l.type, lod);
                ShadowMaps[light] = { lod, -1 };
            }
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            if (l.type == ShadowType::Point) {
                UpdatePoint(l, transform.position, GetSettingsForLOD(lod), l.depthMap);
            }
            if (l.type == ShadowType::Directional) {
                UpdateDirectional(l, transform.position, GetSettingsForLOD(lod), l.depthMap);
            } 
            glCullFace(GL_BACK); 
        };


        for (auto& e : closestGroupLights.point) renderLight(e);
        for (auto& e : closestGroupLights.directional) renderLight(e);
        int updateIndex = GEngine->time->GetFrameCount() % 3;
        if (updateIndex == 0) {
            //for (auto& e : closestGroupLights.directional) renderLight(e);
        }
        else if (updateIndex == 1) {
            //for (auto& e : closestGroupLights.point) renderLight(e);
        }
        else if (updateIndex == 3) {
            //lightInActive.clear();
        }

        SetLightsToSSBO();
    }

    static inline std::unordered_map<Entity, std::pair<ShadowLOD, unsigned int>> ShadowMaps;  // Mapeia luz para textura de sombra
    static inline std::vector<Entity> lightInActive;

    // Lógica para devolver as sombras que não estão mais ativas à pool
    void HandleShadowMapReturn(const glm::vec3& playerPosition) {
        // Verificar se uma luz não está mais nas luzes próximas e devolver o shadowMap para a pool
        std::vector<Entity> toErase;
        for (auto& [lightEntity, lodMapShadow] : ShadowMaps) {
            auto& transform = GEngine->scene->GetComponent<Transform>(lightEntity);
            auto& light = GEngine->scene->GetComponent<LightComponent>(lightEntity);

            // Verificar se a luz está dentro do alcance
            ShadowLOD currentLod = ComputeLOD(glm::distance(transform.position, playerPosition)); 
            if (currentLod != lodMapShadow.first) {  //lod == ShadowLOD::NONE || lod != lodMapShadow
                std::cout << " lod atualizado "   << std::endl;
                // Devolver o shadow map à pool se a luz estiver fora do alcance
                ReturnShadowMapToPool(currentLod, light.depthMap, light.type);
                light.depthMap = 0;  // Limpar o depthMap da luz
                toErase.push_back(lightEntity);
            }
        }
        // Excluir os elementos marcados
        for (auto& it : toErase) ShadowMaps.erase(it); 
    }

    static int SetSunToShader(Shader* shader) {
        //PROBLEMA DA FORMA COMO ESCREVI SHADERS DO FORWARD NAO TERAM ACESSO A SOMBRAS DO SOL E NEM LUZ DELE 
        if (currentSun == INVALID_ENTITY) return 0;

        shader->use();
        shader->setFloat("far_plane", 7.5f);     //shader->setFloat("far_plane", light.range);
        auto& light = GEngine->scene->GetComponent<LightComponent>(currentSun);
        auto& transform = GEngine->scene->GetComponent<Transform>(currentSun);
          
        std::string prefix = "light";   
        
        shader->setVec3(prefix + ".position", transform.rotation);  //ERADO TEM Q SER DIREÇAO

        //shader->setVec3(prefix + ".color", light.color * light.intensity);
        shader->setVec3(prefix + ".color", light.color);
        shader->setInt(prefix + ".type", static_cast<int>(light.type)); 
        shader->setInt(prefix + ".castS", 1); 
        shader->setVec3(prefix + ".direction", transform.GetForwardDirection());
        shader->setMat4(prefix + ".lightMatrix", light.lightSpaceMatrix); 
        //shader->setInt(prefix + ".indexMap", light.depthMap); 

        shader->setInt("shadowMapSun", 4); // começa depois das 2D   
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, light.depthMap);     //light.depthMap 

        //light.textureIndex = textureUnit; // <- sincroniza o índice
        //shader.setInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
        return 1;
    }

    static void SetPointsToShader(Shader* shader) {
        // Enviar para o shader
        shader->use();
        for (auto& [lightEntity, map] : ShadowMaps) {
            int lightCount = map.second;
            auto& light = GEngine->scene->GetComponent<LightComponent>(lightEntity);
            if (light.type == ShadowType::Point) {
                shader->setInt("pointShadows[" + std::to_string(lightCount) + "]", lightCount); // começa depois das 2D  
                glActiveTexture(GL_TEXTURE0 + lightCount);
                glBindTexture(GL_TEXTURE_CUBE_MAP, light.depthMap);     //light.depthMap 
                //std::cout << lightCount << " uauuuuu " << lightCount << std::endl;
            } 
        } 
        shader->setFloat("far_plane", 25.0f); 
        shader->setInt("numLights", ShadowMaps.size());
        //shader->setInt("albedoMap", 0);
        shader->setVec3("viewPos", GEngine->mainCamera->Position);
    }

    static void SetLightsToShader(Shader* shader) {
        // Enviar para o shader
        shader->use();
        for (auto& [lightEntity, map] : ShadowMaps) {
            int lightCount = map.second;
            auto& light = GEngine->scene->GetComponent<LightComponent>(lightEntity); 
            if (light.type == ShadowType::Point) {
                shader->setInt("pointShadows[" + std::to_string(lightCount) + "]", lightCount); // começa depois das 2D  
                glActiveTexture(GL_TEXTURE0 + lightCount);
                glBindTexture(GL_TEXTURE_CUBE_MAP, light.depthMap);     //light.depthMap 
                //std::cout << lightCount << " uauuuuu " << lightCount << std::endl;
            }
            else {
                shader->setInt("shadowMaps[" + std::to_string(lightCount) + "]", lightCount); // começa depois das 2D
                glActiveTexture(GL_TEXTURE0 + lightCount);
                glBindTexture(GL_TEXTURE_2D, light.depthMap);     //light.depthMap  
            }
        }

        //shader->setFloat("far_plane", light.range);
        shader->setFloat("far_plane", 25.0f);

        shader->setInt("numLights", ShadowMaps.size());  
        shader->setInt("albedoMap", 0);
        shader->setVec3("viewPos", GEngine->mainCamera->Position);
    }
     

    void SetLightsToSSBO() {      
        // Carregar dados das luzes em SSBO
        glBindBuffer(GL_UNIFORM_BUFFER, lightSSBO);
        int shadowMapIndexPoint = 0; // Usado para determinar o índice do shadow map
        int shadowMapIndexDir = 0; // Usado para determinar o índice do shadow map
        int i = 0;
         
        for (auto lightEntity : lightInActive) {           //lights
            auto& light = GEngine->scene->GetComponent<LightComponent> (lightEntity);
            auto& transform = GEngine->scene->GetComponent<Transform>  (lightEntity);
             
            // Usar diretamente o LightComponent, sem criar uma nova estrutura   
            // Enviar tipo da luz (ShadowType como int)
            glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 0, sizeof(int), &light.type);
            // Enviar posição da luz (glm::vec3)
            //EXCLUSIVO PARA POINTS , PQ PARA DIR PRECISA SER A ROTAÇAO
            glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 16, sizeof(glm::vec3), glm::value_ptr(transform.position));
            // Enviar cor da luz (glm::vec3)
            glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 32, sizeof(glm::vec3), &light.color);
            // Enviar intensidade da luz (float)
            glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 48, sizeof(float), &light.intensity);
            // Enviar alcance da luz (float)
            glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 52, sizeof(float), &light.range);
            // Enviar ângulo da luz (float) para Spot
            glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 56, sizeof(float), &light.angle);
            // Enviar se a luz projeta sombra (bool, mas enviado como 1 byte)
            glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 60, sizeof(bool), &light.castShadow);
            // Enviar matriz de espaço da luz (glm::mat4)
            glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 64, sizeof(glm::mat4), glm::value_ptr(light.lightSpaceMatrix));
               
            int shadowMapIdx = -1;
            if (light.depthMap != 0 ) {
                if (light.type == ShadowType::Directional) { 
                    shadowMapIdx = shadowMapIndexDir++;
                    ShadowMaps[lightEntity].second = shadowMapIdx;    // Atribui o índice do shadow map para a luz 
                }
                else {
                    shadowMapIdx = shadowMapIndexPoint++;
                    ShadowMaps[lightEntity].second = shadowMapIdx;
                }
            }
            // Enviar o depthMap (GLuint) 
                //std::cout << i << " uauuuuu " << shadowMapIdx << std::endl;
            glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 128, sizeof(GLuint), &shadowMapIdx);        //&light.depthMap
             
            //shadowMaps.push_back(light.depthMap); // Adiciona o depthMap à lista de shadowMaps
             
            i++;
        } 
        glBindBuffer(GL_UNIFORM_BUFFER, 0);  
    }
     
    std::vector<Entity> SelectLightsToCastShadow(const std::vector<Entity>& lights); 
    LightGroups GroupLightsByType(const std::vector<Entity>& lights);
    std::vector<Entity> GetClosestLights(const glm::vec3& playerPos);

    static inline Entity currentSun = INVALID_ENTITY;  // Armazena a referência do sol atual, para garantir que tenha apenas um  
private: 
    GLuint depthMapFBO = 0; 
    GLuint lightSSBO;
    unsigned int offBuff;

    std::vector<ShadowSettings> poolShadowsTex;                       //std::vector<std::pair<ShadowLOD, ShadowSettings>> poolShadowsTex;
    /*static Shader* shadowShader;
    static Shader* pointShadowShader;*/

    void HandleSunChange(); 
    // Lógica para atualizar luz direcional
    void UpdateDirectional(LightComponent& light, const glm::vec3& pos, unsigned int resolution, GLuint Texture); 
    // Lógica para atualizar luz pontual
    void UpdatePoint(LightComponent& light, const glm::vec3& pos, unsigned int resolution, GLuint Texture);

    ShadowLOD ComputeLOD(float distance);
    unsigned int GetSettingsForLOD(ShadowLOD lod); 
    void ReturnShadowMapToPool(ShadowLOD lod, GLuint shadowMap, ShadowType type);

    GLuint GetAvailableShadowMap(ShadowType type, ShadowLOD lod);

    void verify();
    //ShadowSettings* GetShadowSettings(ShadowLOD lod);
};



//// Enviar as 6 transformações de sombra (glm::mat4) para luzes de tipo Spot/Point
//for (int j = 0; j < 6; ++j) {
//    glBufferSubData(GL_SHADER_STORAGE_BUFFER, i * 192 + 80 + j * sizeof(glm::mat4), sizeof(glm::mat4), &light.shadowTransforms[j]);
//}





//struct mapShadow {
//    GLuint id;
//    ShadowLOD lod;
//};





























//inline static std::vector<GLuint> shadowMapsDir;
//inline static std::vector<GLuint> shadowMapsPoint;
//void SetLightsToSSBO() {
//    // Pega todas as luzes
//    //auto lights = GEngine->scene->GetEntitiesWith<LightComponent, Transform>();
//
//    // Cria uma lista para armazenar os dados das luzes
//    std::vector<LightComponent> lightData;
//    std::unordered_map<LightComponent*, int> shadowMapIndicesPoint;  // Lista de índices de shadow maps
//    std::unordered_map<LightComponent*, int> shadowMapIndicesDir;  // Lista de índices de shadow maps
//
//    int shadowMapIndexPoint = 0; // Usado para determinar o índice do shadow map
//    int shadowMapIndexDir = 0; // Usado para determinar o índice do shadow map
//
//    shadowMapsDir.clear();
//    shadowMapsPoint.clear();
//    for (auto lightEntity : lightInActive) {           //lights
//        auto& light = GEngine->scene->GetComponent<LightComponent>(lightEntity);
//        auto& transform = GEngine->scene->GetComponent<Transform>(lightEntity);
//
//        // Usar diretamente o LightComponent, sem criar uma nova estrutura
//        lightData.push_back(light);
//
//        if (light.depthMap != 0) {
//            if (light.type == ShadowType::Directional) {
//                shadowMapsDir.push_back(light.depthMap); // Adiciona o depthMap à lista de shadowMaps
//                // Atribui o índice do shadow map para a luz
//                shadowMapIndicesDir[&light] = shadowMapIndexDir++; // Incrementa o índice para o próximo shadow map
//            }
//            else {
//                shadowMapsPoint.push_back(light.depthMap); // Adiciona o depthMap à lista de shadowMaps 
//                shadowMapIndicesPoint[&light] = shadowMapIndexPoint++; // Incrementa o índice para o próximo shadow map  
//            }
//        }
//        //shadowMaps.push_back(light.depthMap); // Adiciona o depthMap à lista de shadowMaps
//
//    }
//
//    // Carregar dados das luzes em SSBO
//    glBindBuffer(GL_UNIFORM_BUFFER, lightSSBO);
//    for (int i = 0; i < lightData.size(); ++i) {
//        const LightComponent& light = lightData[i];
//
//        // Enviar tipo da luz (ShadowType como int)
//        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 0, sizeof(int), &light.type);
//        // Enviar posição da luz (glm::vec3)
//        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 16, sizeof(glm::vec3), glm::value_ptr(light.position));
//        // Enviar cor da luz (glm::vec3)
//        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 32, sizeof(glm::vec3), glm::value_ptr(glm::vec3(0, 1, 1)));
//        // Enviar intensidade da luz (float)
//        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 48, sizeof(float), &light.intensity);
//        // Enviar alcance da luz (float)
//        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 52, sizeof(float), &light.range);
//        // Enviar ângulo da luz (float) para Spot
//        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 56, sizeof(float), &light.angle);
//        // Enviar se a luz projeta sombra (bool, mas enviado como 1 byte)
//        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 60, sizeof(bool), &light.castShadow);
//        // Enviar matriz de espaço da luz (glm::mat4)
//        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 64, sizeof(glm::mat4), glm::value_ptr(light.lightSpaceMatrix));
//
//        int shadowMapIdx = -1;
//        if (light.type == ShadowType::Point) {
//            if (shadowMapIndicesPoint.find(&lightData[i]) != shadowMapIndicesPoint.end()) {
//                int shadowMapIdx = shadowMapIndicesPoint[&lightData[i]];  // Pega o índice do shadow map correspondente
//                std::cout << "dasdas";
//            }
//        }
//        else if (shadowMapIndicesDir.find(&lightData[i]) != shadowMapIndicesDir.end()) {
//            int shadowMapIdx = shadowMapIndicesDir[&lightData[i]];  // Pega o índice do shadow map correspondente 
//        }
//        // Enviar o depthMap (GLuint) 
//        glBufferSubData(GL_UNIFORM_BUFFER, i * offBuff + 128, sizeof(GLuint), &shadowMapIdx);        //&light.depthMap
//
//    }
//    glBindBuffer(GL_UNIFORM_BUFFER, 0);
//
//}