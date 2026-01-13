#pragma once 
#include <imgui.h>
#include "../../Engine/Instancing/MaterialInstance.h"
#include "../../Engine/Materials/MaterialBase.h"
#include "../../Engine/Materials/TextureCache.h"
#include "../../Engine/Assets/MaterialAsset.h"

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



    static void DrawTexture(
        const std::string& name,
        MaterialInstance::Value& v,
        MaterialInstance& inst
    ) {
        ImGui::Text("%s", name.c_str());

        ImGui::SameLine(120);
        ImGui::BeginGroup();

        auto tex = std::get_if<std::shared_ptr<Texture>>(&v.data);
        ImGui::Image(
            (ImTextureID)(intptr_t)((tex && *tex) ? (*tex)->ID : 0),
            ImVec2(64, 64)
        );

        ImGui::SameLine();

        ImGui::BeginGroup();
        if (ImGui::Button("Load Texture")) {
            // abrir file dialog
            // auto path = FileDialog::Open(...)
            // v.data = TextureCache::Get().Load(path);
        } 
        //ImGui::SameLine();

        if (ImGui::Button("Solid Color")) {
            v.data = glm::vec4(1, 1, 1, 1);
        }
        ImGui::EndGroup();

        ImGui::EndGroup();
    }


    static void DrawTextureOrColor(
        const char* label,
        MaterialInstance::Value& v,
        MaterialInstance& inst
    ) {
        ImGui::TextUnformatted(label);
        ImGui::SameLine(140);

        ImGui::BeginGroup();
         
        // Garantia: sempre Texture
        /*if (!std::holds_alternative<std::shared_ptr<Texture>>(v.data)) {
            v.data = TextureCache::Get().GetSolidColor({ 1,1,1,1 });
            inst.dirty = true;
        }*/

        if (std::holds_alternative<std::shared_ptr<Texture>>(v.data)) {  

            auto& tex = std::get<std::shared_ptr<Texture>>(v.data);
            ImTextureID id = tex ? (ImTextureID)(intptr_t)tex->ID : 0;

            ImGui::Image(id, ImVec2(64, 64));
            ImGui::SameLine();

            ImGui::BeginGroup();
            if (ImGui::Button("Load")) {
                // TODO file dialog
                // v = TextureCache::Load(...)
                inst.dirty = true;
            }

            if (ImGui::Button("Clear")) {
                v.data = glm::vec4(1, 1, 1, 1); // vira cor
                inst.dirty = true;
            }
            ImGui::EndGroup();
        }
        else if (std::holds_alternative<glm::vec4>(v.data)) {

            auto& color = std::get<glm::vec4>(v.data);
            ImGui::SetNextItemWidth(-1);

            if (ImGui::ColorEdit4( (std::string("##") + label).c_str(), &color.x )) {
                v.data = color;
                inst.dirty = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Texture")) {
                v.data = std::shared_ptr<Texture>{};
                inst.dirty = true;
            }
        }

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

            if (def.useSlider)
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
            DrawTextureOrColor(name.c_str(), *value, inst);     //DrawTexture(name, *value, inst);
            break;
        }

        ImGui::PopID();
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