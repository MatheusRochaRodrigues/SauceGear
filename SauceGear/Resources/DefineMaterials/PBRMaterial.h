#pragma once
#include "Material.h"
#include "../Core/EngineContext.h"
#include "../Core/Time.h"

class PBRMaterial : public MaterialBase {
public: 
    PBRMaterial() : MaterialBase(nullptr) {  // chama construtor do pai              //PBRMaterial(std::shared_ptr<Shader> sh) : MaterialBase(sh) {
        // nenhuma stateful value aqui — só convenções de nomes e binds
        tag = RenderTag::Deferred;
        DefineParameters(); // Define parâmetros padrão
    }

    // Parâmetros comuns PBR
    void DefineParameters() override {
        nativeParams["Albedo"]   .fallback = glm::vec3(1.0f);
        nativeParams["Metallic"] .fallback = 0.0f;
        nativeParams["Roughness"].fallback = 0.5f;
        nativeParams["AO"]       .fallback = 1.0f; 
    }

    // garante samplers padronizados na ordem que o shader espera (opcional) // ex: define binding indices conhecidos (mas nosso ApplyToShader já usa TEX units dinamicamente) // Ex: tempo de animação para emissive blinking
    void BaseBindings(Shader* sh) const override {
         //float t = static_cast<float>(GEngine->time->GetTime());
        //sh->setFloat("uTime", t);
    }
};
















//class EmissivePBRMaterial : public PBRMaterial {
//public:
//    EmissivePBRMaterial(std::shared_ptr<Shader> sh) : PBRMaterial(sh) {
//        // nada fixo: a instância vai carregar albedo/metal/rough etc e emissive params extra
//    }
//
//    // Pode sobrescrever ApplyBaseBindings para binds extra se necessário
//}; 

 