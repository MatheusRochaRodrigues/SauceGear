#pragma once
#include "../../Graphics/Texture.h"  
#include <unordered_map>
#include <string> 
#include <variant> 

using MatValue = std::variant<std::monostate, float, glm::vec3, glm::vec4, std::shared_ptr<Texture>>; 
struct MaterialParam {
    unsigned int unit;
    MatValue value;
    MatValue fallback; // sempre algo "editável" no inspector       //bool useFallback = true; // se true, ignora value e gera a textura 1x1 do fallback
};

enum class RenderTag {
    Opaque,
    Transparent,
    Forward,
    Deferred
};    

class MaterialBase {
public:
    RenderTag tag = RenderTag::Forward;
    std::string name;             // Nome para binding no shader

    MaterialBase() {
        RenderTag tag = RenderTag::Deferred;
        DefineParameters();
    }

    MaterialBase(std::shared_ptr<Shader> shader, std::shared_ptr<MaterialBase> parent = nullptr) : shader(shader), parent(parent) {
        DefineParameters(); 
        // herda parâmetros do pai
        if (parent) {
            for (auto& [name, param] : parent->nativeParams) {
                if (nativeParams.find(name) == nativeParams.end()) {
                    nativeParams[name] = param; // herda se não sobrescrito
                }
            }
        }  
    }  

    virtual ~MaterialBase() = default;

    // subclasses definem parâmetros default (nomes e tipos)
    virtual void DefineParameters() {
        /*
        // Parâmetros comuns PBR
        nativeParams["Albedo"].fallback = glm::vec3(0.5f, 0.5f, 1.0f);         // 0
        nativeParams["Metallic"].fallback = 0.0f;                   // 1
        nativeParams["Roughness"].fallback = 0.5f;                  // 2
        nativeParams["AO"].fallback = 1.0f;                         // 3
        */
    }

    // Aplica todos os parâmetros no shader (usado pelo renderer - chama ApplyInstance)
    virtual void BaseBindings(Shader* sh) const { /* Hook para binds fixos do material base(por ex.sampler indices padrão) */ } 
    // Hook para binds especiais de cada material (tempo, animação, etc)
    virtual void BindSpecial(Shader* sh) const {}

    // Registra campos para o reflection (exemplo)
    static void RegisterFieldsForReflection() {
        // usar REFLECT_CLASS / REFLECT_FIELD nas classes concretas do editor;
        // aqui deixamos vazio — editor pode iterar params via API pública (GetParam)
    }

    const std::unordered_map<std::string, MaterialParam>& GetDefaultParams() const { return nativeParams; } 
    const std::shared_ptr<Shader>& GetShader() const { return shader; }
    std::shared_ptr<MaterialBase> GetParent() const { return parent; }
    
    void AddParam(unsigned int unit, const std::string& name, MatValue value) {
        MaterialParam param;
        param.value = std::move(value);
        param.unit = unit;
        nativeParams[name] = std::move(param);
    }

    void AddFallParam(unsigned int unit, const std::string& name, MatValue fallback) {
        MaterialParam param;
        param.fallback = std::move(fallback);
        param.unit = unit;
        nativeParams[name] = std::move(param);
    } 
    //inline static unsigned int s_unit = 0;
protected:
    // não armazena valores por si só aqui; valores concretos ficam nas MaterialInstance
    std::shared_ptr<Shader> shader = nullptr;
    std::shared_ptr<MaterialBase> parent;

    std::unordered_map<std::string, MaterialParam> nativeParams; 
};



  





 
