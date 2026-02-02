#include "LightPass.h"
#include "../../Core/EngineContext.h"
#include "../../Core/Camera.h"
#include "../../Scene/SceneECS.h"

#include "ShadowPool.h"
#include "LightMakers/LightDirectional.h"
#include "LightMakers/LightPoint.h"
#include "LightMakers/CascadeSun.h" 

using Scene = SceneECS;
using namespace HandleLights;

struct GPULight {
    glm::vec4 pos_radius;       // xyz pos | w radius
    glm::vec4 color_intensity;  // rgb color | a intensity
    glm::vec4 params;           // x type | y cast | z angle | w indexMap
    glm::mat4 lightMatrix;
};

// inicialização mínima — a maioria dos recursos está no pool/cascade
LightPass::LightPass() {
    std::cout << "[LightPass] ctor\n"; 
    // cria SSBO para luzes (como antes)   
    glGenBuffers(1, &lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
        MAX_LIGHTS * sizeof(GPULight),
        nullptr,
        GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightSSBO);

    //std::cout << sizeof(GPULight) << std::endl;

    // depthMapFBO será criado/gerenciado em ShadowPool::Init()
    ShadowPool::Init();             // garante FBO/textures pool
    CascadeSun::Init();             // inicializa cascades do sol
} 

void LightPass::Update(bool updtSun) {
    glm::vec3 posPlayer = GEngine->mainCamera->Position;

    // obter as luzes mais próximas que podem lançar sombras
    auto closest    = GetClosestLights(posPlayer);
    auto selected   = SelectLightsToCastShadow(closest);
    LightGroups groups                     = GroupLightsByType(selected);

    // Limpa os arrays que vamos preencher
    lightInActive.point.clear();   lightInActive.directional.clear();   lightInActive.spot.clear();
    //temp teste
    lightInActive.point = groups.point;

    //------------------------------------------------ ** SUN LIGHT ** --------------------------------------------------------
    // Atualiza o sol (cascades) se existir
    if ( currentSun && (currentSun != INVALID_ENTITY) ) {
        // pegamos componente do sol
        auto& transform = GEngine->scene->GetComponent<TransformComponent>(currentSun);
        auto& light = GEngine->scene->GetComponent<LightComponent>(currentSun);

        if (light.castShadow && light.type == LightType::Directional) { 
            if (updtSun) {
                CascadeSun::UpdateSunShadow(light, transform);            //lightInActive.directional.push_back(currentSun);   
                  
                // Atualiza shadowCascadeLevels local para shaders
                shadowCascadeLevels = CascadeSun::GetCascadeDistances(); 
            }
        }

    } else {
        // tenta encontrar um directional para ser sun (preserva comportamento anterior)
        auto entities = GEngine->scene->GetEntitiesWith<LightComponent, TransformComponent>();
        for (auto& ent : entities) {
            auto& lc = GEngine->scene->GetComponent<LightComponent>(ent);
            if (lc.type == LightType::Directional) { currentSun = ent;      break; }
        }
    } 

    //------------------------------------------------ ** POINT LIGHT ** -------------------------------------------------------- 
    //// Para cada luz selecionada (exceto o sol tratado acima) atualiza shadowmaps
    //for (auto& e : selected) {
    //    if (e == currentSun) continue;
    //    auto& transform = GEngine->scene->GetComponent<TransformComponent>(e);
    //    auto& l = GEngine->scene->GetComponent<LightComponent>(e);

    //    ShadowLOD lod = ComputeLOD(glm::distance(transform.position, posPlayer));
    //    if (lod == ShadowLOD::NONE) continue;

    //    /*
    //    if (l.depthMap == 0) {
    //        l.depthMap = ShadowPool::GetAvailableShadowMap(l.type, lod);
    //        ShadowPool::ShadowMaps[e] = { lod, -1 };
    //    }
    //    */

    //    glEnable(GL_CULL_FACE);      glCullFace(GL_FRONT); 
    //    if (l.type == LightType::Point) {
    //        LightPoint::UpdatePoint(l, transform.position, ShadowPool::GetSettingsForLOD(lod), ShadowPool::depthMapFBO, l.depthMap);
    //        lightInActive.point.push_back(e);
    //    }
    //    else if (l.type == LightType::Directional) {
    //        LightDirectional::UpdateDirectional(l, transform.position, ShadowPool::GetSettingsForLOD(lod), ShadowPool::depthMapFBO, l.depthMap);
    //        lightInActive.directional.push_back(e);
    //    } 
    //    glCullFace(GL_BACK);
    //}

    // devolve sombras que saíram do alcance
    //HandleShadowMapReturn(posPlayer);



    // finalmente escrever dados para SSBO
    SetLightsToSSBO(); 
}  


void LightPass::SetLightsToSSBO() { 
    std::vector<GPULight> gpuLights; 
    for(auto& e : lightInActive.point) {
        if (!GEngine->scene->EntityExists(e)) continue;
        if (!GEngine->scene->HasComponent<LightComponent>(e) ||
            !GEngine->scene->HasComponent<TransformComponent>(e)) continue;

        auto& l = GEngine->scene->GetComponent<LightComponent>(e);
        auto& t = GEngine->scene->GetComponent<TransformComponent>(e);

        GPULight g{};
        g.pos_radius = { t.position, l.range };
        g.color_intensity = { l.color, l.intensity };

        float shadowIndex = -1.0f; // valor seguro padrão
        auto it = ShadowPool::ShadowMaps.find(e);
        if (it != ShadowPool::ShadowMaps.end()) shadowIndex = float(it->second.second); 

        g.params = {
            float(l.type),
            float(l.castShadow),
            l.angle,
            shadowIndex
        };
        g.lightMatrix = l.lightSpaceMatrix;

        gpuLights.push_back(g);
    }  
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);

    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, gpuLights.size() * sizeof(GPULight), gpuLights.data() ); 
     
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); //default
} 
/*
DICA FINAL (engine-level) 
No shader:

if (light.params.w < 0.0)  // luz sem shadow 
else   // usa shadow map 
*/


void LightPass::HandleShadowMapReturn(const glm::vec3& playerPosition) {
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

        auto& transform = GEngine->scene->GetComponent<TransformComponent>(lightEntity);
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

template<typename T>
void EraseFromVector(std::vector<T>& v, T value) {
    v.erase(std::remove(v.begin(), v.end(), value), v.end());
} 



void LightPass::OnEntityDestroyed(Entity e) {

    // Remove de listas ativas
    EraseFromVector(lightInActive.point, e);
    EraseFromVector(lightInActive.directional, e);
    EraseFromVector(lightInActive.spot, e);

    // Remove shadow map
    auto it = ShadowPool::ShadowMaps.find(e);
    if (it != ShadowPool::ShadowMaps.end()) {

        auto& [lod, _] = it->second;

        if (GEngine->scene->HasComponent<LightComponent>(e)) {
            auto& light = GEngine->scene->GetComponent<LightComponent>(e);
            if (light.depthMap != 0) {
                ShadowPool::ReturnShadowMapToPool(lod, light.depthMap, light.type);
                light.depthMap = 0;
            }
        }

        ShadowPool::ShadowMaps.erase(it);
    }
}



