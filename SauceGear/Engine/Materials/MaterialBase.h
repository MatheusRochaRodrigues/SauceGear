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
        // shader expectation
        enum Type               { Float, Vec3, Vec4, Texture };     // that's define what shader expects to receive 
        Type type;

        enum class UIType       { Drag, Slider, Color, NONE };            // This defines how the user edits.          , Texture
        UIType specification;

        uint32_t unit = 0;     // só usado se shaderType == Texture, logo se for textura


        float min = 0.0f;
        float max = 1.0f;
        //bool useSlider = false;
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