#pragma once
#include "../../Graphics/Texture.h"  
#include <unordered_map>
#include <string> 
#include <variant> 

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
    virtual void DefineParameters() {}

    // Aplica todos os parâmetros no shader (usado pelo renderer - chama ApplyInstance)
    virtual void ApplyBaseBindings(Shader* sh) const {
        // Hook para binds fixos do material base (por ex. sampler indices padrão)
    }
     
    // Hook para binds especiais de cada material (tempo, animação, etc)
    virtual void ApplySpecialBindings(Shader* sh) const {}

    // Registra campos para o reflection (exemplo)
    static void RegisterFieldsForReflection() {
        // usar REFLECT_CLASS / REFLECT_FIELD nas classes concretas do editor;
        // aqui deixamos vazio — editor pode iterar params via API pública (GetParam)
    }

    const std::unordered_map<std::string, MaterialParam>& GetDefaultParams() const {
        return nativeParams;
    }

    const std::shared_ptr<Shader>& GetShader() const { return shader; }
    std::shared_ptr<MaterialBase> GetParent() const { return parent; }
      
protected:
    // não armazena valores por si só aqui; valores concretos ficam nas MaterialInstance
    std::shared_ptr<Shader> shader;
    std::shared_ptr<MaterialBase> parent;

    std::unordered_map<std::string, MaterialParam> nativeParams; 
};



  





 
