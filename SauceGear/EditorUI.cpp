//template<typename T>
//void DrawComponent(const std::string& name, SceneECS& scene, Entity entity) {
//    if (scene.HasComponent<T>(entity)) {
//        if (ImGui::TreeNode(name.c_str())) {
//            T& component = scene.GetComponent<T>(entity);
//
//            // Aqui você renderiza os campos reais do componente.
//            // Exemplo genérico:
//            if constexpr (std::is_same_v<T, Transform>) {
//                ImGui::DragFloat3("Position", &component.position.x);
//                ImGui::DragFloat3("Rotation", &component.rotation.x);
//                ImGui::DragFloat3("Scale", &component.scale.x);
//            }
//
//            ImGui::TreePop();
//        }
//    }
//}
