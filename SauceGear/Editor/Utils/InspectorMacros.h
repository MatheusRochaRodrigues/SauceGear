#pragma once
//primitives
#define DRAW_VEC3 (obj, field)  ImGuiUtils::DragVec3 (#field, obj.field)
#define DRAW_FLOAT(obj, field)  ImGuiUtils::DragFloat(#field, obj.field)
#define DRAW_INT  (obj, field)  ImGuiUtils::DragInt  (#field, obj.field)
#define DRAW_BOOL (obj, field)  ImGuiUtils::Checkbox (#field, obj.field)
//generic

//structs
#define DRAW_TREE (label, body) ImGuiUtils::BeginTree(label, [&]() body)
