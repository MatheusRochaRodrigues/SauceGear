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

        auto tex = std::get_if<std::shared_ptr<Texture>>(&v.data);
        ImGui::Image(
            (ImTextureID)(intptr_t)((tex && *tex) ? (*tex)->ID : 0),
            ImVec2(64, 64)
        );

        if (ImGui::Button("Load Texture")) {
            // abrir file dialog
            // auto path = FileDialog::Open(...)
            // v.data = TextureCache::Get().Load(path);
        }

        ImGui::SameLine();

        if (ImGui::Button("Solid Color")) {
            v.data = glm::vec4(1, 1, 1, 1);
        }
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
    static void DrawParam(
        const std::string& name,
        const MaterialBase::ParamDef& def,
        MaterialInstance& inst,
        MaterialAsset& asset
    ) {
        MaterialInstance::Value* value = nullptr;

        if (inst.overrides.count(name))
            value = &inst.overrides[name];
        else if (asset.defaults.count(name))
            value = &asset.defaults[name];
        else
            return;

        ImGui::PushID(name.c_str());

        switch (def.type) {
        case MaterialBase::ParamDef::Float:
            DrawFloat(name, *value);
            break;

        case MaterialBase::ParamDef::Vec3:
            DrawVec3(name, *value);
            break;

        case MaterialBase::ParamDef::Vec4:
            DrawColor(name, *value);
            break;

        case MaterialBase::ParamDef::Texture:
            DrawTexture(name, *value, inst);
            break;
        }

        ImGui::PopID();
    }
};
