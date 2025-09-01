#pragma once
#include <imgui.h>
#include <glm/glm.hpp>
#include <string>
#include <functional>

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

    inline void DragVec3(const char* label, glm::vec3& value) {
        ImGui::DragFloat3(label, &value.x, 0.1f);
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
}




//template<typename Func>
//inline void BeginTree(const char* label, Func&& content) {
//    if (ImGui::TreeNode(label)) {
//        content();
//        ImGui::TreePop();
//    }
//}