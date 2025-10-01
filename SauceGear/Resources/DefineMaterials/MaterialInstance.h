#pragma once
#include "Material.h"
#include "TextureCache.h" 
#include <unordered_map> 
#include "../ECS/Reflection/Meta.h" // teu reflect (opcional) 
#include <variant>        // std::variant, std::get, std::get_if, std::holds_alternative, std::monostate
#include <memory>         // std::shared_ptr
#include <glm/glm.hpp>    // glm::vec3, glm::vec4
 
 
class MaterialInstance {
public: 
    MaterialInstance(std::shared_ptr<MaterialBase> base) : base(base) {
        if (!this->base) {
            std::cerr << "[Apply] Erro: base is nullptr in Material Instance\n";
            throw std::runtime_error("MaterialInstance criado sem base!");
        } 
        MatParams = base->GetDefaultParams(); // já clona defaults  
        // build param types map for quick lookup (used para decidir se um float deve virar 1x1 texture)
        //for (const auto& def : base->GetParamDefs()) paramTypes[def.name] = def.type;
    }
    //MaterialInstance() {
    //    base = std::make_shared<PBRMaterial>();         //std::make_shared<PBRMaterial>(); 
    //    if (!this->base) {
    //        std::cerr << "[Apply] Erro: base is nullptr in Material Instance\n";
    //        throw std::runtime_error("MaterialInstance criado sem base!");
    //    }
    //    std::cout << std::endl << std::endl << std::endl << " dsa aq = " << base->GetDefaultParams().size() << std::endl;
    //    MatParams = base->GetDefaultParams(); // já clona defaults 
    //    std::cout << std::endl << std::endl << std::endl << " dsa22 aq = " << MatParams.size() << std::endl;
    //    std::cout << std::endl << std::endl << " dgb aq = " << base->dbg << std::endl;
    //    
    //    //for (auto& [name, param] : base->GetDefaultParams()) { //MatParams[name] = param; //} // copia defaults do pai
    //}

    /*
    // chama ApplyBaseBindings de pai primeiro, depois filho
    void ApplyBaseBindingsChain(const MaterialBase* mat, Shader* sh) {
        if (!mat) return;
        if (mat->GetParent()) ApplyBaseBindingsChain(mat->GetParent().get(), sh);
        mat->ApplyBaseBindings(sh);
    }
    void ApplySpecialBindingsChain(const MaterialBase* mat, Shader* sh) {
        if (!mat) return;
        if (mat->GetParent()) ApplySpecialBindingsChain(mat->GetParent().get(), sh);
        mat->ApplySpecialBindings(sh);
    }
    */
      
    void ApplyBindingsBaseChain(const MaterialBase* mat, Shader* shader, int depth = 0) {
        if (!mat) return;  
        // safety: limit recursion depth to detect cycles / runaway recursion
        if (depth > 64) {
            std::cerr << "[ApplyBindingsBaseChain] recursion depth exceeded (possible cycle). Stopping.\n";
            return;
        } 
        try { 
            // primeiro aplica os do pai 
            if (auto parentShared = mat->GetParent()) {     // copy the shared_ptr
                // detect self-parent (common bug)
                if (parentShared.get() == mat) {
                    std::cerr << "[ApplyBindingsBaseChain] ERROR: parent points to self. Breaking to avoid infinite recursion.\n";
                }
                else { 
                    ApplyBindingsBaseChain(parentShared.get(), shader, depth + 1); 
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "[ApplyBindingsBaseChain] exception when reading parent: " << e.what() << "\n";
            return;
        } catch (...) {
            std::cerr << "[ApplyBindingsBaseChain] unknown exception when reading parent\n";
            return;
        } 
        // depois aplica os do material atual
        mat->BaseBindings(shader);
    }  
      
    void Apply(Shader* overrideShader = nullptr) {
        Shader* active = nullptr;
        if (!base) {
            std::cerr << "[Apply] Erro: base é nullptr\n";
            return;
        } 

        if (overrideShader) active = overrideShader; 
        else if (base->tag == RenderTag::Forward) active = base->GetShader().get(); 
         
        if (!active) {  // Ex: PBRMaterial (deferred) → usa o shader do pipeline
            std::cerr << "[Apply] Erro: Shader ativo é nullptr\n";
            return;
        } 
        active->use();

        // aplica os "base bindings" do pai até o filho    
        ApplyBindingsBaseChain(base.get(), active);         
       
        //Binds
        for (auto& [name, param] : MatParams) { 
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
            // Textures
            std::shared_ptr<Texture> texPtr; 
            if (auto tex = std::get_if<std::shared_ptr<Texture>>(&param.value)) {
                texPtr = *tex; // textura definida
            } 

            //Fallbacks
            else if (!texPtr && !std::holds_alternative<std::monostate>(param.fallback)) {
                // fallback como textura 1x1
                if (auto pVec3 = std::get_if<glm::vec3>(&param.fallback)) { 
                    texPtr = TextureCache::Get().GetSolidColor(glm::vec4(*pVec3, 1.0f)); 
                }
                else if (auto pVec4 = std::get_if<glm::vec4>(&param.fallback)) {
                    texPtr = TextureCache::Get().GetSolidColor(*pVec4);                     //resolveTexture(*pVec4);
                }
                else if (auto pFloat = std::get_if<float>(&param.fallback)) { 
                    texPtr = TextureCache::Get().GetFloat(*pFloat);
                }
                if(texPtr) param.value = texPtr;
            }

            // Bind da textura 
            if (texPtr) { 
                texPtr->Bind(param.unit);               //glActiveTexture(GL_TEXTURE0 + param.unit);  
                active->setInt(name.c_str(), param.unit);
            } 
        } 
        // Binds especiais do material (tempo, animação, etc)  
        base->BindSpecial(active); 
    }

    // Registra campos para o reflection (exemplo)
    static void RegisterFieldsForReflection() {
        // usar REFLECT_CLASS / REFLECT_FIELD nas classes concretas do editor;
        // aqui deixamos vazio — editor pode iterar params via API pública (GetParam)
    }  
     
    void Bind() { Apply(); } 
    const std::shared_ptr<MaterialBase>& GetBase() const { return base; }
    // Reflection-friendly getters (editor usa)
    MaterialParam& GetParam(const std::string& name) { return MatParams[name]; }
    // Seta parâmetro generico (texture)
    MaterialParam& SetTexture(const std::string& name, std::shared_ptr<Texture> tex) { MatParams[name].value = tex; return MatParams[name]; }
    MaterialParam& SetFloat(const std::string& name, float v) { MatParams[name].value = v; return MatParams[name]; }
    MaterialParam& SetVec3(const std::string& name, const glm::vec3& v) { MatParams[name].value = v; return MatParams[name]; }
    // Fallback setters (editor vai usar esses campos via reflection)
    MaterialParam& SetFallbackColor(const std::string& name, const glm::vec3& c) { MatParams[name].fallback = c; return MatParams[name]; }
    MaterialParam& SetFallbackFloat(const std::string& name, float f) { MatParams[name].fallback = f; return MatParams[name]; }
    MaterialParam& SetFallbackTexture(const std::string& name, std::shared_ptr<Texture> t) { MatParams[name].fallback = t; return MatParams[name]; }
    void SetUnit(const std::string& name, unsigned int unit) { MatParams[name].unit = unit; }

private:
    std::shared_ptr<MaterialBase> base;
    std::unordered_map<std::string, MaterialParam> MatParams; //overrides
     
    /*void ApplyFallback(MaterialParam& param, Shader* sh, const std::string& name, int& texUnit) {
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
    }*/

    // Texturas transitórias criadas a cada Apply (1x1 fallbacks) para manter alive
    //std::vector<std::shared_ptr<Texture>> transientTextures;
};



 