#pragma once 
#include <imgui.h>
#include "../../Engine/Instancing/MaterialInstance.h"
#include "../../Engine/Materials/MaterialBase.h"
#include "../../Engine/Materials/TextureCache.h"
#include "../../Engine/Assets/MaterialAsset.h"
#include "../FileDialog/FileDialog.h"

struct MaterialInspector {

    static void DrawFloat(const std::string& name, MaterialInstance::Value& v) {
        float val = std::get<float>(v.data);
        if (ImGui::DragFloat(name.c_str(), &val, 0.01f))
            v.data = val;
    }

    static void DrawVec3(const std::string& name, MaterialInstance::Value& v) {
        glm::vec3 val = std::get<glm::vec3>(v.data);
        if (ImGui::ColorEdit3(name.c_str(), &val.x))
            v.data = val;
    }

    static void DrawColor(const std::string& name, MaterialInstance::Value& v) {
        glm::vec4 val = std::get<glm::vec4>(v.data);
        if (ImGui::ColorEdit4(name.c_str(), &val.x))
            v.data = val;
    } 

    static void SanitizeMaterialValue(MaterialInstance::Value& v) {
        // Caso antigo: glm::vec4 → solid color texture
        if (std::holds_alternative<glm::vec4>(v.data)) {
            v.data = TextureCache::Get().GetSolidColor(
                std::get<glm::vec4>(v.data)
            );
            return;
        }

        // Caso vazio / não inicializado
        if (!std::holds_alternative<std::shared_ptr<Texture>>(v.data)) {
            v.data = TextureCache::Get().GetSolidColor({ 1,1,1,1 });
            return;
        }

        // Caso texture nula
        auto& tex = std::get<std::shared_ptr<Texture>>(v.data);
        if (!tex) {
            tex = TextureCache::Get().GetSolidColor({ 1,1,1,1 });
        }
    }

    static std::shared_ptr<Texture> MakeScalarTexture(float v) {
        return TextureCache::Get().GetSolidColor({ v, v, v, 1.0f });
    }


    static void DrawFieldTexture(
        const char* label,
        MaterialInstance::Value& v,
        const MaterialBase::ParamDef& def,
        MaterialInstance& inst
    ) {
        // 🔒 SANITIZA ANTES DE QUALQUER USO
        SanitizeMaterialValue(v);

        ImGui::TextUnformatted(label);
        ImGui::SameLine(140);

        ImGui::BeginGroup();

        auto& tex = std::get<std::shared_ptr<Texture>>(v.data);
        ImTextureID id = tex ? (ImTextureID)(intptr_t)tex->ID : 0;

        ImGui::Image(id, ImVec2(64, 64), ImVec2(0, 1)/*uv0*/, ImVec2(1, 0)/*uv1*/);
        ImGui::SameLine();

        ImGui::BeginGroup();

        if (ImGui::Button("Load")) {
            std::string path = FileDialog::Open("*.png;*.jpg");
            if (!path.empty()) {
                v.data = TextureCache::Get().Load(path);
                inst.dirty = true;
            }
        } 

        // Solid color editor
        if (tex && tex->isSolidColor) {
            ImGui::SetNextItemWidth(120); 

            float scalar = tex->dataColor.x;

            string name = std::string("##") + label; 
            switch (def.specification) {

            case MaterialBase::ParamDef::UIType::Color:
                glm::vec4 color = tex->dataColor;
                if (ImGui::ColorEdit4(name.c_str(), &color.x)) {    //(std::string("##") + label).c_str()
                    v.data = TextureCache::Get().GetSolidColor(color);
                    inst.dirty = true;
                }
            break;


            case MaterialBase::ParamDef::UIType::Slider:
                //DrawLabeledFloat(name.c_str(), v, def.min, def.max);     // ImGui::SliderFloat(name.c_str(), &v, def.min, def.max); 
                if (ImGui::SliderFloat( (std::string("##scalar_") + label).c_str(),  &scalar, 0.0f, 1.0f )) {
                    v.data = MakeScalarTexture(scalar);
                    inst.dirty = true;
                }
                tex->dataColor.x = scalar;
            break; 


            case MaterialBase::ParamDef::UIType::Drag: 
                ImGui::DragFloat(name.c_str(), &scalar, 0.01f);
                inst.dirty = true;  
                tex->dataColor.x = scalar;

                break;

            }
        }

        if (ImGui::Button("Reset")) {
            v.data = TextureCache::Get().GetSolidColor({ 1,1,1,1 });
            inst.dirty = true;
        }

        ImGui::EndGroup();
        ImGui::EndGroup();
    }


    static void Draw(MaterialInstance& inst) {

        if (!inst.asset || !inst.asset->base) {
            ImGui::TextDisabled("Invalid Material");
            return;
        }

        auto& base = *inst.asset->base;

        ImGui::TextDisabled("Material: %s", inst.asset->name.c_str());
        ImGui::Separator();

        for (auto& [name, def] : base.layout) {
            DrawParam(name, def, inst, *inst.asset);
        }

    }

private:
    static void DrawLabeledFloat(
        const char* label, float& value,
        float min, float max
    ) {
        ImGui::TextUnformatted(label);

        ImGui::SameLine(120);              // coluna fixa do label
        ImGui::SetNextItemWidth(-1);       // ocupa o resto da linha

        ImGui::SliderFloat(
            (std::string("##") + label).c_str(),
            &value,
            min,
            max
        );
    }


    static void DrawParam(
        const std::string& name,
        const MaterialBase::ParamDef& def,
        MaterialInstance& inst,
        MaterialAsset& asset
    ) {
        MaterialInstance::Value* value = nullptr;

        if      (inst.overrides.count(name)) value = &inst.overrides[name];
        else if (asset.defaults.count(name)) value = &asset.defaults[name];
        else    return;

        ImGui::PushID(name.c_str());

        switch (def.type) {
        case MaterialBase::ParamDef::Float: {
            float v = std::get<float>(value->data);

            if (def.specification == MaterialBase::ParamDef::UIType::Slider)
                DrawLabeledFloat(name.c_str(), v, def.min, def.max);     // ImGui::SliderFloat(name.c_str(), &v, def.min, def.max);
            else
                ImGui::DragFloat(name.c_str(), &v, 0.01f);

            value->data = v;
            break;
        } 

        case MaterialBase::ParamDef::Vec3:
            DrawVec3(name, *value);
            break;

        case MaterialBase::ParamDef::Vec4:
            DrawColor(name, *value);
            break;

        case MaterialBase::ParamDef::Texture: 
            DrawFieldTexture(name.c_str(), *value, def, inst);     //DrawTexture(name, *value, inst);
            //if (name == "Albedo") { ImGui::Checkbox("SRGB", &inst.isSRGB);          ImGui::Separator(); }
            break;
        }

        ImGui::PopID();
    }

    // ================================
    // UI helpers (Unity style columns)
    // ================================
    static void BeginRow(const char* label) {
        ImGui::TextUnformatted(label);
        ImGui::SameLine(140);
        ImGui::SetNextItemWidth(-1);
    }

};





/*

Use layout plano para atributos - Exemplo correto
 
if (ImGui::CollapsingHeader("Material")) {

    DrawLabeledFloat("Metallic", metallic, 0.0f, 1.0f);
    DrawLabeledFloat("Roughness", roughness, 0.0f, 1.0f);

}  

Sem TreeNode → sem empurrar tudo para a direita.
 

 if (ImGui::CollapsingHeader("Materials")) {
    for (size_t i = 0; i < renderer.materials.size(); i++) {
        auto& mat = renderer.materials[i];

        ImGui::Text("Slot %zu", i);
        ImGui::SameLine();
        ImGui::Button(mat->asset->name.c_str());

        if (ImGui::IsItemClicked()) {
            Inspector::Select(mat); // 🔥
        }
    }
} 


*/



/*


        case MaterialBase::ParamDef::UIType::Slider: {
            BeginRow(name.c_str());

            float v = std::get<float>(value->data);
            if (ImGui::SliderFloat( ("##" + name).c_str(), &v,  def.min,
                def.max
            )) {
                value->data = v;
                inst.dirty = true;
            }
            break;
        }

        case MaterialBase::ParamDef::UIType::Drag: {
            BeginRow(name.c_str());

            float v = std::get<float>(value->data);
            if (ImGui::DragFloat(("##" + name).c_str(), &v, 0.01f)) {
                value->data = v;
                inst.dirty = true;
            }
            break;
        }

        case MaterialBase::ParamDef::UIType::Color: {
            BeginRow(name.c_str());

            glm::vec4 c = std::get<glm::vec4>(value->data);
            if (ImGui::ColorEdit4(("##" + name).c_str(), &c.x)) {
                value->data = c;
                inst.dirty = true;
            }
            break;
        }

        case MaterialBase::ParamDef::UIType::Texture: {
            BeginRow(name.c_str());
            DrawTextureSlot(name.c_str(), *value, inst);
            break;
        }



*/