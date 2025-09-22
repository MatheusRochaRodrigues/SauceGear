#pragma once
#include "Material.h"
#include "MaterialUtils.h" 
#include <unordered_map> 
#include "../ECS/Reflection/Meta.h" // teu reflect (opcional) 
#include <variant>        // std::variant, std::get, std::get_if, std::holds_alternative, std::monostate
#include <memory>         // std::shared_ptr
#include <glm/glm.hpp>    // glm::vec3, glm::vec4
 
 
class MaterialInstance {
public:
    inline static int sdbg2 = 0;
    int dbg2 = 0;
    MaterialInstance(std::shared_ptr<MaterialBase> base) : base(base) {
        if (!this->base) {
            std::cerr << "[Apply] Erro: base is nullptr in Material Instance\n";
            throw std::runtime_error("MaterialInstance criado sem base!");
        }
        std::cout << std::endl << std::endl << std::endl << " dsa aq = " << base->GetDefaultParams().size() << std::endl;
        MatParams = base->GetDefaultParams(); // já clona defaults
        std::cout << std::endl << std::endl << " dgb aq = " << base->dbg << std::endl;
         
        dbg2 = sdbg2++;
        std::cout << std::endl<< " debug = " << dbg2 << std::endl;

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
    void ApplyBindingsBaseChain(const MaterialBase* mat, Shader* shader, int depth = 0) {
        if (!mat) {
            std::cout << "[ApplyBindingsBaseChain] mat == nullptr\n";
            return;
        }
        if (!shader) {
            std::cerr << "[ApplyBindingsBaseChain] ERROR: shader == nullptr\n";
            return;
        }
        auto parentShared = mat->GetParent();
        if (!parentShared) {
            std::cout << "[ApplyBindingsBaseChain] parent == nullptr\n";
        }
        else {
            std::cout << "[ApplyBindingsBaseChain] parent OK\n";
        }

        // safety: limit recursion depth to detect cycles / runaway recursion
        if (depth > 64) {
            std::cerr << "[ApplyBindingsBaseChain] recursion depth exceeded (possible cycle). Stopping.\n";
            return;
        }

        // Print diagnostic: addresses and (if possible) parent pointer info
        std::cout << "[ApplyBindingsBaseChain] mat: " << static_cast<const void*>(mat)
            << " depth=" << depth << "\n";

        // try-catch around access in case of exceptions (not typical for raw pointer deref,
        // but keeps controlled prints)
        try {
            auto parentShared = mat->GetParent(); // copy the shared_ptr
            if (parentShared) {
                std::cout << "[ApplyBindingsBaseChain] parent: " << static_cast<const void*>(parentShared.get())
                    << " use_count=" << parentShared.use_count() << "\n";

                // detect self-parent (common bug)
                if (parentShared.get() == mat) {
                    std::cerr << "[ApplyBindingsBaseChain] ERROR: parent points to self. Breaking to avoid infinite recursion.\n";
                }
                else {
                    ApplyBindingsBaseChain(parentShared.get(), shader, depth + 1);
                }
            }
            else {
                std::cout << "[ApplyBindingsBaseChain] parent == nullptr\n";
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[ApplyBindingsBaseChain] exception when reading parent: " << e.what() << "\n";
            return;
        }
        catch (...) {
            std::cerr << "[ApplyBindingsBaseChain] unknown exception when reading parent\n";
            return;
        }

        // depois aplica os do material atual
        std::cout << "[ApplyBindingsBaseChain] calling ApplyBaseBindings on " << static_cast<const void*>(mat) << "\n";
        mat->ApplyBaseBindings(shader);
    }
    */
    
    void ApplyBindingsBaseChain(const MaterialBase* mat, Shader* shader) {
        if (!mat) return; 
        // primeiro aplica os do pai
        std::cout << "g911132    " << (mat->GetParent() != nullptr) << std::endl;
        if (mat->GetParent() != nullptr) {
            std::cout << "g9111111" << std::endl;
            ApplyBindingsBaseChain(mat->GetParent().get(), shader);
            std::cout << "g9211111" << std::endl;
        }
        std::cout << "gf92332" << std::endl;
        // depois aplica os do material atual
        mat->BaseBindings(shader);
    } 

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
         
        std::cout << "before ApplyBindingsBaseChain parent ptr: " << base.get() << " shader: " << active << "\n";
        //ApplyBindingsBaseChain(base.get(), active);
        std::cout << "after ApplyBindingsBaseChain\n";
        // aplica os "base bindings" do pai até o filho            
         
        int texUnit = 0;
        std::cout << "2131";
        std::cout << std::endl << std::endl << " dgb aq = " << base->dbg << std::endl;
        std::cout << std::endl << " debug = " << dbg2 << std::endl;
        std::cout << "params.size() = " << MatParams.size() << std::endl;
        for (auto& [name, param] : MatParams) {
            std::cerr << "222";
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
            else if (!std::holds_alternative<std::monostate>(param.fallback)) {
                // fallback como textura 1x1
                if (auto pVec3 = std::get_if<glm::vec3>(&param.fallback)) {
                    texPtr = resolveTexture(glm::vec4(*pVec3, 1.0f));        //tex = TextureCache::Get().GetSolidColor(glm::vec4(std::get<glm::vec3>(param.fallback), 1.0f));
                }
                else if (auto pVec4 = std::get_if<glm::vec4>(&param.fallback)) {
                    texPtr = resolveTexture(*pVec4);
                }
                else if (auto pFloat = std::get_if<float>(&param.fallback)) {
                    texPtr = resolveTexture(glm::vec4(*pFloat, *pFloat, *pFloat, 1.0f));
                }
            }
            // Bind da textura
            if (texPtr) {
                glActiveTexture(GL_TEXTURE0 + texUnit);
                texPtr->Bind();
                active->setInt(name, texUnit++);
            } 
        } 
        // Binds especiais do material (tempo, animação, etc)  
        //base->BindSpecial(active);
        std::cerr << "222sav";
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
    void SetTexture(const std::string& name, std::shared_ptr<Texture> tex) { MatParams[name].value = tex; }
    void SetFloat(const std::string& name, float v) { MatParams[name].value = v; }
    void SetVec3(const std::string& name, const glm::vec3& v) { MatParams[name].value = v; }
    // Fallback setters (editor vai usar esses campos via reflection)
    void SetFallbackColor(const std::string& name, const glm::vec3& c) { MatParams[name].fallback = c; }
    void SetFallbackFloat(const std::string& name, float f) { MatParams[name].fallback = f; }
    void SetFallbackTexture(const std::string& name, std::shared_ptr<Texture> t) { MatParams[name].fallback = t; }

private:
    std::shared_ptr<MaterialBase> base;
    std::unordered_map<std::string, MaterialParam> MatParams; //overrides
     
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

    // Texturas transitórias criadas a cada Apply (1x1 fallbacks) para manter alive
    //std::vector<std::shared_ptr<Texture>> transientTextures;
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