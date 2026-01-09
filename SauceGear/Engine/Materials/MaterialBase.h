#pragma once
#include "../Graphics/Shader.h"  
#include <unordered_map>
#include <string> 
#include <variant>  

enum class MaterialDomain {
    Deferred,
    Forward,
    Transparent
};

class MaterialBase {
public:
    MaterialDomain domain;
    Shader* shader = nullptr;

    struct ParamDef {
        enum Type { Float, Vec3, Vec4, Texture };
        Type type;
        uint32_t unit;
    };

    std::unordered_map<std::string, ParamDef> layout;
    virtual ~MaterialBase() = default;
};







//std::string shaderPath;
//if (shaderFileChanged(base->shaderPath)) {
//    base->shader->Reload();
//}
 
/*

PBRMaterial
 ├─ define layout
 ├─ define shaders
 └─ define pipeline

MaterialAsset (.gmat)
 ├─ escolhe o MaterialBase
 └─ define valores padrão

MaterialInstance
 ├─ copia defaults
 └─ sobrescreve valores por entidade

MaterialBinder
 ├─ resolve prioridade
 └─ garante fallback

MeshRenderer
 ├─ aponta Mesh
 └─ aponta MaterialInstance

Renderer
 └─ chama MaterialBinder + Draw


*/



/*

Arquivo .material (JSON)
        ↓
MaterialSerializer
        ↓
MaterialAsset        ← asset "imutável", dados default
        ↓ Instantiate()
MaterialInstance     ← runtime, mutável, por entidade
        ↓
MeshRenderer.Draw()

*/