//#include "../ECS/Components/Transform.h"
//#include "../Utils/InspectorRegistry.h"
//#include "../Utils/ImGuiUtils.h"
//#include "../Utils/InspectorMacros.h" 
//
//void DrawTransform(Entity e, SceneECS& scene) {
//    auto& t = scene.GetComponent<Transform>(e);
//
//    DRAW_TREE("Transform", {
//        DRAW_VEC3(t, position);
//        DRAW_VEC3(t, rotation);
//        DRAW_VEC3(t, scale);
//    });
//}
//
//// Registrar automaticamente
//REGISTER_COMPONENT_DRAWER(Transform, DrawTransform)

 

//#include "../ECS/Components/Transform.h"
//#include "../Utils/InspectorRegistry.h"
//#include <imgui.h>
//
//namespace ComponentDrawers {
//
//    inline void DrawTransform(Entity entity, SceneECS& scene) {
//        auto& t = scene.GetComponent<Transform>(entity);
//
//        if (ImGui::TreeNode("Transform")) {
//            ImGui::DragFloat3("Position", &t.position.x, 0.1f);
//            ImGui::DragFloat3("Rotation", &t.rotation.x, 0.1f);
//            ImGui::DragFloat3("Scale", &t.scale.x, 0.1f);
//            ImGui::TreePop();
//        }
//    }
//
//    // Registrar automaticamente
//    struct TransformDrawerRegister {
//        TransformDrawerRegister() {
//            ComponentInspectorRegistry::Get().RegisterDrawer<Transform>(DrawTransform);
//        }
//    };
//    static TransformDrawerRegister transformDrawerRegister;
//
//} // namespace ComponentDrawers
