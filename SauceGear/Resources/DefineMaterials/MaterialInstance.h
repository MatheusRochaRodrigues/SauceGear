#pragma once
#include "Material.h"
#include "MaterialUtils.h" 
#include <unordered_map> 
#include "../ECS/Reflection/Meta.h" // teu reflect (opcional) 
#include <variant>        // std::variant, std::get, std::get_if, std::holds_alternative, std::monostate
#include <memory>         // std::shared_ptr
#include <glm/glm.hpp>    // glm::vec3, glm::vec4
 
using MatValue = std::variant<std::monostate, float, glm::vec3, glm::vec4, std::shared_ptr<Texture>>;

struct MaterialParam {
    MatValue value;
    MatValue fallback; // sempre algo "editável" no inspector       //bool useFallback = true; // se true, ignora value e gera a textura 1x1 do fallback
};
 
class MaterialInstance {
public:
    MaterialInstance(std::shared_ptr<MaterialBase> base) : base(base) {
        if (base) {
            params = base->GetDefaultParams(); // já clona defaults
        }
    }

    void ApplyBindingsBaseChain(const MaterialBase* mat, Shader* shader) {
        if (!mat) return; 
        // primeiro aplica os do pai
        if (mat->GetParent()) {
            ApplyBindingsBaseChain(mat->GetParent().get(), shader);
        } 
        // depois aplica os do material atual
        mat->ApplyBaseBindings(shader);
    } 
      
    void Apply(Shader* overrideShader = nullptr) {
        Shader* active = overrideShader ? overrideShader : base->GetShader().get();
        if (!active) return;
        active->use();

        // Aplica binds do material base
        ApplyBindingsBaseChain(base.get(), active);         // aplica os "base bindings" do pai até o filho            

        int texUnit = 0;
        for (auto& [name, param] : params) {
            // se é float/vec3, envia direto como uniform (se shader espera float/vec3)
            if (auto pFloat = std::get_if<float>(&param.value)) {
                active->setFloat(name.c_str(), *pFloat);
                continue;
            }
            if (auto pVec3 = std::get_if<glm::vec3>(&param.value)) {
                active->setVec3(name.c_str(), *pVec3);
                continue;
            }
            if (auto pVec4 = std::get_if<glm::vec4>(&param.value)) {
                active->setVec4(name.c_str(), *pVec4);
                continue;
            }

            auto tex = std::get_if<std::shared_ptr<Texture>>(&param.value);
            if (tex && *tex) {
                if (*tex) { // verifica se não é nullptr
                    // Usa a textura definida      //auto tex = std::get<std::shared_ptr<Texture>>(param.value); //glActiveTexture(GL_TEXTURE0 + texUnit);
                    (*tex)->Bind();
                    active->setInt(name, texUnit++);
                }
            } else {
                if (std::holds_alternative<std::monostate>(param.fallback)) continue; //flag to know if exists some fallback

                // Usa fallback como textura 1x1  
                if (auto pVec3 = std::get_if<glm::vec3>(&param.fallback)) {
                    tex = &resolveTexture(glm::vec4(*pVec3, 1.0f));      //tex = TextureCache::Get().GetSolidColor(glm::vec4(std::get<glm::vec3>(param.fallback), 1.0f));
                }
                if (auto pVec4 = std::get_if<glm::vec4>(&param.fallback)) {
                    tex = &resolveTexture(*pVec4);
                }
                if (auto pFloat = std::get_if<float>(&param.fallback)) {
                    tex = &resolveTexture(glm::vec4(*pFloat, *pFloat, *pFloat, 1));
                } 

                if (tex) {
                    glActiveTexture(GL_TEXTURE0 + texUnit);
                    (*tex)->Bind();
                    active->setInt(name, texUnit++);
                }
            } 
        }

        // Binds especiais do material (tempo, animação, etc)  
        base->ApplySpecialBindings(active);
    }

    // Reflection-friendly getters (editor usa)
    MaterialParam& GetParam(const std::string& name) { return params[name]; }
    const std::shared_ptr<MaterialBase>& GetBase() const { return base; } 

    // Seta parâmetro generico (texture)
    void SetTexture(const std::string& name, std::shared_ptr<Texture> tex) {
        params[name].value = tex;
    }
    void SetFloat(const std::string& name, float v) {
        params[name].value = v;
    }
    void SetVec3(const std::string& name, const glm::vec3& v) {
        params[name].value = v;
    }

    // Fallback setters (editor vai usar esses campos via reflection)
    void SetFallbackColor(const std::string& name, const glm::vec3& c) {
        params[name].fallback = c;
    }
    void SetFallbackFloat(const std::string& name, float f) {
        params[name].fallback = f;
    }
    void SetFallbackTexture(const std::string& name, std::shared_ptr<Texture> t) {
        params[name].fallback = t;
    }

    // Registra campos para o reflection (exemplo)
    static void RegisterFieldsForReflection() {
        // usar REFLECT_CLASS / REFLECT_FIELD nas classes concretas do editor;
        // aqui deixamos vazio — editor pode iterar params via API pública (GetParam)
    } 

    void Bind() {
        Apply();
    }

private:
    std::shared_ptr<MaterialBase> base;
    std::unordered_map<std::string, MaterialParam> params;

    // Texturas transitórias criadas a cada Apply (1x1 fallbacks) para manter alive
    //std::vector<std::shared_ptr<Texture>> transientTextures;



    void ApplyFallback(MaterialParam& param, Shader* sh, const std::string& name, int& texUnit) {
        std::shared_ptr<Texture> tex = nullptr;
        if (auto pVec3 = std::get_if<glm::vec3>(&param.fallback)) {
            tex = resolveTexture(glm::vec4(*pVec3, 1.0f));
        }
        else if (auto pVec4 = std::get_if<glm::vec4>(&param.fallback)) {
            tex = resolveTexture(*pVec4);
        }
        else if (auto pFloat = std::get_if<float>(&param.fallback)) {
            tex = resolveTexture(glm::vec4(*pFloat, *pFloat, *pFloat, 1.0f));
        }
        if (tex) {
            tex->Bind(texUnit);
            sh->setInt(name, texUnit++);
        }
    }
};






    /*
    // Chamar no GeometryPass: aplica todos os parametros ao shader (resolve fallbacks e textures 1x1)
    void ApplyToShader(Shader* overrideShader = nullptr) {
        Shader* sh = overrideShader ? overrideShader : (base->GetShader().get());
        if (!sh) return;
        sh->use();

        base->ApplyBaseBindings(sh); // permite binds fixos da família de material

        int texUnit = 0;
        // percorre params e faz bind/uni
        for (auto& [name, pv] : params) {
            // se valor é texture
            if (auto pTex = std::get_if<std::shared_ptr<Texture>>(&pv.value)) {
                if (pTex && *pTex) {
                    glActiveTexture(GL_TEXTURE0 + texUnit);
                    (*pTex)->Bind();
                    sh->setInt(name.c_str(), texUnit++);
                    continue;
                }
            }

            // se é float/vec3, envia direto como uniform (se shader espera float/vec3)
            if (auto pFloat = std::get_if<float>(&pv.value)) {
                sh->setFloat(name.c_str(), *pFloat);
                continue;
            }
            if (auto pVec3 = std::get_if<glm::vec3>(&pv.value)) {
                sh->setVec3(name.c_str(), *pVec3);
                continue;
            }

            // Se chegou aqui: não há valor concreto -> usar fallback (converter para texture 1x1)
            if (pv.useFallbackTexture) {
                // determinístico cache key
                // se shader espera sampler2D para esse nome, enviamos texture 1x1; para floats/vec3 já enviados acima
                std::shared_ptr<Texture> fallbackTex;
                // decide comparar se há fallbackFloat ou fallbackColor (usuario decide via editor)
                // se fallbackFloat != 0 ou usuario quer, cria float texture
                // heurística: se fallbackFloat está diferente de 0..1? só usar se explicitado; aqui sempre suportamos ambos
                // preferencia: se parâmetro nome contém "metal" ou "rough" ou "ao" -> usar fallbackFloat
                bool preferFloat = (name.find("metal") != std::string::npos) ||
                    (name.find("rough") != std::string::npos) ||
                    (name.find("ao") != std::string::npos);

                if (preferFloat) {
                    fallbackTex = Create1x1FloatTexture(pv.fallbackFloat);
                }
                else {
                    fallbackTex = Create1x1ColorTexture(pv.fallbackColor);
                }
                // bind
                glActiveTexture(GL_TEXTURE0 + texUnit);
                fallbackTex->Bind();
                sh->setInt(name.c_str(), texUnit++);
                // store temporary to keep alive this frame (cache global opcional) - aqui podemos push pra um container temporário
                transientTextures.push_back(fallbackTex);
            }
            else {
                // se não usar fallback, set uniform 0/vec3(0) para evitar undefined behaviour
                sh->setFloat(name.c_str(), pv.fallbackFloat);
            }
        }
        // ao terminar de usar, limpa transientTextures se quiser liberar depois (shared_ptr fará cleanup)
        transientTextures.clear();
    }
    */