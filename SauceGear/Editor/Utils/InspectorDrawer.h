#pragma once
#include "../ECS/Reflection/Meta.h"
#include "ImGuiUtils.h"
#include <typeinfo>
#include <functional>
#include <glm/glm.hpp>
#include <string>

namespace InspectorDrawer {

    inline void DrawProperty(const FieldInfo& field, void* instance) {
        switch (field.kind) {
        case FieldKind::Value: {
            char* base = static_cast<char*>(instance);
            void* ptr = base + field.offset;

            if      (field.type == typeid(float)) ImGuiUtils::DragFloat(field.name.c_str(), *static_cast<float*>(ptr));
            else if (field.type == typeid(int))   ImGuiUtils::DragInt(field.name.c_str(), *static_cast<int*>(ptr));
            else if (field.type == typeid(bool))  ImGuiUtils::Checkbox(field.name.c_str(), *static_cast<bool*>(ptr));
            else if (field.type == typeid(glm::vec2)) ImGuiUtils::DragVec2Colored(field.name.c_str(), *static_cast<glm::vec2*>(ptr));
            else if (field.type == typeid(glm::vec3)) ImGuiUtils::DragVec3Colored(field.name.c_str(), *static_cast<glm::vec3*>(ptr));
            else if (field.type == typeid(glm::vec4)) ImGuiUtils::DragVec4(field.name.c_str(), *static_cast<glm::vec4*>(ptr));
            // Aqui você pode adicionar enums, std::string, etc
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

    inline void DrawType(const char* label, void* instance, TypeInfo* type) {
        ImGuiUtils::BeginTree(label, [&]() {
            for (auto& field : type->fields) {
                DrawProperty(field, instance);
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
}






//inline void DrawType(const char* label, void* instance, TypeInfo* type) {
//    ImGuiUtils::BeginTree(label, [&]() {
//        for (auto& field : type->fields) {
//            DrawProperty(field, instance);
//        }
//    });
//}
//
//// Desenha todos os componentes de uma entidade
//template<typename SceneType>
//inline void DrawEntity(SceneType& scene, Entity entity) {
//    for (auto& compType : scene.GetComponentTypes(entity)) {
//        if (auto* typeInfo = ReflectionRegistry::Get().Get(compType.name)) {
//            void* compPtr = scene.GetComponentRaw(entity, compType);
//            InspectorDrawer::DrawType(compType.name.c_str(), compPtr, typeInfo);
//        }
//    }
//}