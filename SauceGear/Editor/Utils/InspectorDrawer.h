#pragma once
#include "ImGuiUtils.h"
#include "../../Engine/Data/Color.h"
#include "../../Engine/ECS/Reflection/Meta.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <typeindex>
#include <cstring>
#include <functional>

// --- Nota: extensão mínima em TypeInfo para suportar callback ---
// Adicione esse member em sua definição TypeInfo (no arquivo onde TypeInfo está definido):
// std::function<void(void* instance)> onEdited = nullptr;
// (Se você já tem TypeInfo em outro arquivo, acrescente apenas este campo.)

namespace InspectorDrawer {

    /*
    // Helper: test and update a glm::quat field via ImGui (shows Euler in degrees)
    inline bool DrawQuatAsEuler(const std::string& label, glm::quat* qptr) {
        if (!qptr) return false;
        glm::quat q = *qptr;
        // Convert to euler (radians) then to degrees for UI
        glm::vec3 euler = glm::degrees(glm::eulerAngles(q)); // yaw/pitch/roll ordering from glm
        // Normalize angles to -180..180 for better UX
        for (int i = 0; i < 3; ++i) {
            if (euler[i] > 180.0f) euler[i] -= 360.0f * std::floor((euler[i] + 180.0f) / 360.0f);
            if (euler[i] < -180.0f) euler[i] += 360.0f * std::ceil((-euler[i] + 180.0f) / 360.0f);
        }

        // ImGuiUtils DragVec3 functions in your code appear to accept references; adapt if needed.
        bool changed = ImGuiUtils::DragVec3Colored(label.c_str(), euler);

        if (changed) {
            // Convert degrees -> radians -> quaternion
            glm::vec3 radians = glm::radians(euler);
            glm::quat newQ = glm::quat(radians); // glm::quat(glm::vec3) makes quaternion from euler angles
            *qptr = glm::normalize(newQ);
            return true;
        }
        return false;
    }
    inline void DrawProperty(const FieldInfo& field, void* instance, TypeInfo* ownerType = nullptr) {
        switch (field.kind) {
        case FieldKind::Value: {
            char* base = static_cast<char*>(instance);
            void* ptr = base + field.offset;

            // Primitivos e vetores
            if (field.type == typeid(float))   ImGuiUtils::DragFloat(field.name.c_str(), *static_cast<float*>(ptr));
            else if (field.type == typeid(int))     ImGuiUtils::DragInt(field.name.c_str(), *static_cast<int*>(ptr));
            else if (field.type == typeid(bool))    ImGuiUtils::Checkbox(field.name.c_str(), *static_cast<bool*>(ptr));
            else if (field.type == typeid(glm::vec2)) ImGuiUtils::DragVec2Colored(field.name.c_str(), *static_cast<glm::vec2*>(ptr));
            else if (field.type == typeid(glm::vec3)) ImGuiUtils::DragVec3Colored(field.name.c_str(), *static_cast<glm::vec3*>(ptr));
            else if (field.type == typeid(glm::vec4)) ImGuiUtils::DragVec4(field.name.c_str(), *static_cast<glm::vec4*>(ptr));

            // --- QUATERNION handling: show as Euler degrees and write back into quaternion ---
            else if (field.type == typeid(glm::quat)) {
                glm::quat* qptr = static_cast<glm::quat*>(ptr);
                bool changed = DrawQuatAsEuler(field.name, qptr);

                // se tiver mudado, dispare callback do tipo para sinalizar (por ex. Transform::MarkDirty)
                if (changed && ownerType && ownerType->onEdited) {
                    ownerType->onEdited(instance);
                }
            }
            // Strings e outros tipos podem ser implementados aqui...
            if (field.type == typeid(std::string)) {
                std::string* s = static_cast<std::string*>(ptr);
                char buf[512];
                std::strncpy(buf, s->c_str(), sizeof(buf));
                buf[sizeof(buf) - 1] = '\0';
                if (ImGui::InputText(field.name.c_str(), buf, sizeof(buf))) {
                    *s = std::string(buf);
                    if (ownerType && ownerType->onEdited) ownerType->onEdited(instance);
                }
            } 

            break;
        }
        case FieldKind::Header:
            ImGuiUtils::Header(field.name);
            break;
        case FieldKind::Space:
            ImGuiUtils::Space();
            break;
        }
    }
    */
    inline bool DrawQuatAsEuler(const std::string& label, glm::quat* qptr) {
        if (!qptr) return false;

        glm::vec3 euler = glm::degrees(glm::eulerAngles(*qptr));

        // Normaliza ângulos para -180..180
        for (int i = 0; i < 3; ++i) {
            if (euler[i] > 180.0f) euler[i] -= 360.0f * std::floor((euler[i] + 180.0f) / 360.0f);
            if (euler[i] < -180.0f) euler[i] += 360.0f * std::ceil((-euler[i] + 180.0f) / 360.0f);
        }

        bool changed = ImGuiUtils::DragVec3Colored(label.c_str(), euler);

        if (changed) {
            *qptr = glm::normalize(glm::quat(glm::radians(euler)));
        }
        return changed;
    }

    //DrawField                         DrawVec3Field
    inline void DrawProperty(const FieldInfo& field, void* instance, TypeInfo* ownerType = nullptr) {
        switch (field.kind) {
        case FieldKind::Value: {
            char* base = static_cast<char*>(instance);
            void* ptr = base + field.offset;                        //equivalent -> void* fieldPtr = (char*)componentPtr + field.offset;
            bool changed = false;
             
            // ===== ENUM =====
            if (field.enumInfo) {
                int* value = static_cast<int*>(ptr); 
                switch (field.widget) {

                case EditorWidget::EnumCombo: {
                    std::vector<const char*> labels;
                    for (auto& v : field.enumInfo->values)
                        labels.push_back(v.label.c_str());

                    int current = *value;
                    if (ImGui::Combo(field.name.c_str(), &current, labels.data(), (int)labels.size())) {
                        *value = current;
                        changed = true;
                    }
                    break;
                } 
                case EditorWidget::EnumRadio:  break; 
                case EditorWidget::EnumButtons:  break; 
                case EditorWidget::EnumFlags:  break;
                }
            } 

            if      (field.type == typeid(float)) {
                float& v = *static_cast<float*>(ptr); 
                if (field.widget == EditorWidget::SliderFloat) {
                    changed |= ImGui::SliderFloat(
                        field.name.c_str(), &v,  field.min,  field.max
                    );
                } else {
                    changed |= ImGuiUtils::DragFloat(field.name.c_str(), v);
                }
            }

            else if (field.type == typeid(int)) {
                int& v = *static_cast<int*>(ptr);

                if (field.widget == EditorWidget::SliderInt) {
                    changed |= ImGui::SliderInt(
                        field.name.c_str(),
                        &v,
                        (int)field.min,
                        (int)field.max
                    );
                }
                else {
                    changed |= ImGuiUtils::DragInt(field.name.c_str(), v);
                }
            }

            else if (field.type == typeid(bool))        
                changed |= ImGuiUtils::Checkbox(field.name.c_str(), *static_cast<bool*>(ptr));
            else if (field.type == typeid(glm::vec2))   
                changed |= ImGuiUtils::DragVec2Colored(field.name.c_str(), *static_cast<glm::vec2*>(ptr)); 

            else if (field.type == typeid(glm::vec3)) {
                glm::vec3& c = *static_cast<glm::vec3*>(ptr);

                if (field.widget == EditorWidget::Color) {
                    if (ImGui::ColorEdit3(
                        field.name.c_str(),
                        &c.x,
                        ImGuiColorEditFlags_HDR |
                        ImGuiColorEditFlags_Float |
                        ImGuiColorEditFlags_AlphaBar
                    )) {
                        changed = true;
                    }
                } else {
                    changed |= ImGuiUtils::DragVec3Colored(field.name.c_str(), c);
                } 
            } 
            
            else if (field.type == typeid(glm::vec4))   
                changed |= ImGuiUtils::DragVec4(field.name.c_str(), *static_cast<glm::vec4*>(ptr));
            else if (field.type == typeid(glm::quat)) {
                changed |= DrawQuatAsEuler(field.name, static_cast<glm::quat*>(ptr));
            }

            // ... strings, enums etc  
            else if (field.type == typeid(std::string)) {
                std::string& str = *static_cast<std::string*>(ptr);

                // buffer temporário local
                char buffer[512];
                std::memset(buffer, 0, sizeof(buffer));

#ifdef _MSC_VER
                strncpy_s(buffer, sizeof(buffer), str.c_str(), _TRUNCATE);
#else
                std::strncpy(buffer, str.c_str(), sizeof(buffer) - 1);
#endif
                 
                if (ImGui::InputText(field.name.c_str(), buffer, sizeof(buffer))) {
                    str = std::string(buffer);
                    changed = true;
                }
            }
            
            else if (field.type == typeid(Color)) {
                Color& c = *static_cast<Color*>(ptr);

                /*float col[4] = { c.r, c.g, c.b, c.a };*/

                if (ImGui::ColorEdit4(
                    field.name.c_str(),
                    &c.value.x,    //col
                    ImGuiColorEditFlags_HDR |
                    ImGuiColorEditFlags_Float |
                    ImGuiColorEditFlags_AlphaBar
                )) {
                    /*c.r = col[0]; c.g = col[1]; c.b = col[2]; c.a = col[3];*/
                    changed = true;
                }
            }

            /*              Aprimorado mas comentei por teste
            else if (field.type == typeid(Color)) {
                Color& c = *static_cast<Color*>(ptr);

                float col[4] = { c.r, c.g, c.b, c.a };

                ImGuiColorEditFlags flags = ImGuiColorEditFlags_Float;

                if (c.space == ColorSpace::HDR)
                    flags |= ImGuiColorEditFlags_HDR;
                if (c.space == ColorSpace::SRGB)
                    flags |= ImGuiColorEditFlags_DisplayRGB;

                if (ImGui::ColorEdit4(
                    field.name.c_str(),
                    col,
                    flags
                )) {
                    c.r = col[0];    c.g = col[1];    c.b = col[2];    c.a = col[3];
                    changed = true;
                }
            }
            */

            if (changed && ownerType && ownerType->onEdited) ownerType->onEdited(instance);     // instance == componentPtr

            break;
        }
        case FieldKind::Header:
            ImGuiUtils::Header(field.name);
            break;
        case FieldKind::Space:
            ImGuiUtils::Space();
            break;

        case FieldKind::Vector: {
            char* base = static_cast<char*>(instance);
            void* vecPtr = base + field.offset;

            size_t count = field.getSize(vecPtr);

            if (ImGui::TreeNode(field.name.c_str())) {

                // Botão Add
                if (ImGui::Button("+")) {
                    field.resize(vecPtr, count + 1);
                }

                ImGui::SameLine();
                if (ImGui::Button("-") && count > 0) {
                    field.resize(vecPtr, count - 1);
                }

                for (size_t i = 0; i < count; ++i) {
                    void* elem = field.getElement(vecPtr, i);

                    std::string label = "##" + std::to_string(i);

                    // reutiliza o mesmo drawer de tipo básico
                    if (field.elementType == typeid(float))
                        ImGuiUtils::DragFloat(label.c_str(), *static_cast<float*>(elem));
                    else if (field.elementType == typeid(glm::vec3))
                        ImGuiUtils::DragVec3Colored(label.c_str(), *static_cast<glm::vec3*>(elem));
                }

                ImGui::TreePop();
            }
            break;
        }

        }


    }



    inline void DrawType(const char* label, void* instance, TypeInfo* type) {
        if (!type || !instance) return;
        ImGuiUtils::BeginTree(label, [&]() {
            for (auto& field : type->fields) {
                DrawProperty(field, instance, type);
            }
            });
    }

    template<typename SceneType>
    inline void DrawEntity(SceneType& scene, Entity entity) {
        for (auto* typeInfo : scene.GetComponentTypes(entity)) {
            void* compPtr = scene.GetComponentRaw(entity, *typeInfo);
            if (compPtr) {
                DrawType(typeInfo->name.c_str(), compPtr, typeInfo);
            }
        }
    }

} // namespace InspectorDrawer
