#pragma once
#include <imgui.h>
#include <glm/glm.hpp>
#include <string>
#include <functional>

inline ImVec4 operator*(const ImVec4& a, const ImVec4& b) {
    return ImVec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

inline ImVec4 operator*(const ImVec4& a, float s) {
    return ImVec4(a.x * s, a.y * s, a.z * s, a.w * s);
}

namespace ImGuiUtils {
     

    inline void DragFloat(const char* label, float& value) {
        ImGui::DragFloat(label, &value, 0.1f);
    }

    inline void DragInt(const char* label, int& value) {
        ImGui::DragInt(label, &value);
    }

    inline void Checkbox(const char* label, bool& value) {
        ImGui::Checkbox(label, &value);
    }

    inline void DragVec2(const char* label, glm::vec2& value) {
        ImGui::DragFloat2(label, &value.x, 0.1f);
    }

    // Função auxiliar para desenhar glm::vec2 com cores 
    inline bool DragVec2Colored(const char* label, glm::vec2& value, float speed = 0.1f, float min = 0.0f, float max = 0.0f) {
        bool changed = false;

        ImGui::PushID(label);

        // Label na esquerda
        ImGui::TextUnformatted(label);
        ImGui::SameLine();

        ImGui::BeginGroup(); // agrupa X e Y na mesma linha

        float fullWidth = ImGui::CalcItemWidth();
        float itemWidth = fullWidth / 2.0f;

        // X (vermelho)
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 100, 255));
        ImGui::SetNextItemWidth(itemWidth);
        changed |= ImGui::DragFloat("##X", &value.x, speed, min, max, "%.3f");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // Y (verde)
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 255, 100, 255));
        ImGui::SetNextItemWidth(itemWidth);
        changed |= ImGui::DragFloat("##Y", &value.y, speed, min, max, "%.3f");
        ImGui::PopStyleColor();

        ImGui::EndGroup();

        ImGui::PopID();

        return changed;
    }


    inline void DragVec3(const char* label, glm::vec3& value) {
        ImGui::DragFloat3(label, &value.x, 0.1f);
    }

    inline bool DragVec3Colored(const char* label, glm::vec3& value, float speed = 0.1f) {
        bool changed = false;

        ImGui::PushID(label);

        // Criar duas colunas: uma para o nome, outra para os controles
        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(0, 100.0f); // largura fixa do rótulo
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();

        const char* axisLabels[3] = { "X", "Y", "Z" };
        const ImVec4 axisColors[3] = {
            ImVec4(0.8f, 0.1f, 0.15f, 1.0f), // Vermelho X
            ImVec4(0.2f, 0.7f, 0.2f, 1.0f),  // Verde Y
            ImVec4(0.1f, 0.25f, 0.8f, 1.0f)  // Azul Z
        };

        float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
        ImVec2 buttonSize(lineHeight + 3.0f, lineHeight);

        for (int i = 0; i < 3; i++) {
            ImGui::PushStyleColor(ImGuiCol_Button, axisColors[i]);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, axisColors[i] * 1.1f);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, axisColors[i] * 0.9f);

            if (ImGui::Button(axisLabels[i], buttonSize)) {
                value[i] = 0.0f;
                changed = true;
            }
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::SetNextItemWidth(70.0f); // largura fixa de cada campo numérico
            changed |= ImGui::DragFloat(("##" + std::string(axisLabels[i])).c_str(), &value[i], speed);

            if (i < 2) ImGui::SameLine();
        }

        ImGui::Columns(1);
        ImGui::PopID();

        return changed;
    }


    inline void DragVec4(const char* label, glm::vec4& value) {
        ImGui::DragFloat4(label, &value.x, 0.1f);
    }

    /*inline void InputString(const char* label, std::string& str) {
        char buffer[256];
        strncpy(buffer, str.c_str(), sizeof(buffer));
        if (ImGui::InputText(label, buffer, sizeof(buffer))) {
            str = buffer;
        }
    }*/

    inline void BeginTree(const char* label, const std::function<void()>& fn) {
        if (ImGui::TreeNode(label)) {
            fn();
            ImGui::TreePop();
        }
    }


    // Desenha um header grande como no Unity
    inline void Header(const std::string& text, ImVec4 Color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f)) {
        ImGui::Separator();
        ImGui::TextColored(Color, "%s", text.c_str());
        ImGui::Separator();
    }

    // Adiciona espaço vertical
    inline void Space(float pixels = 5.0f) {
        ImGui::Dummy(ImVec2(0.0f, pixels));
    }
}




//template<typename Func>
//inline void BeginTree(const char* label, Func&& content) {
//    if (ImGui::TreeNode(label)) {
//        content();
//        ImGui::TreePop();
//    }
//}