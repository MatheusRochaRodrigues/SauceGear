#pragma once
#include "../Components/Transform.h"
#include "../Components/HierarchyComponent.h"
#include "../Scene/SceneECS.h"
#include "../Core/EngineContext.h"
#include "../System.h"
#include <vector> 

// Helper simples para atualizar a árvore de transforms.
// Adapte scene.GetAllEntities() para a sua API de ECS se necessário.

namespace TransformSys {

    // Atualiza recursivamente a subtree, parentWorld deve ser a world matrix do pai.
    inline void UpdateRecursive(SceneECS& scene, Entity entity, const glm::mat4& parentWorld) {
        Transform& t = scene.GetComponent<Transform>(entity);

        // Recalcula localMatrix se necessario e depois worldMatrix
        t.UpdateWorldFromParent(parentWorld);

        // Recurre para filhos
        if (scene.HasComponent<HierarchyComponent>(entity)) {
            HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(entity);
            Entity child = h.firstChild;
            while (child != INVALID_ENTITY) {
                UpdateRecursive(scene, child, t.model);
                HierarchyComponent& childH = scene.GetComponent<HierarchyComponent>(child);
                child = childH.nextSibling;
            }
        }
    }

    // Atualiza todas entidades: encontra raízes (parent == INVALID_ENTITY) e atualiza a árvore
    inline void UpdateAllTransforms(SceneECS& scene) {
        // --- NOTE: adapte esta iteração à sua API ---
        std::vector<Entity> all = scene.GetAllEntities(); // se você não tiver, troque por seu método
        for (Entity e : all) {
            if (!scene.HasComponent<Transform>(e)) continue;

            // root detection: se tem HierarchyComponent e parent != INVALID_ENTITY -> não é root
            bool isRoot = true;
            if (scene.HasComponent<HierarchyComponent>(e)) {
                HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(e);
                if (h.parent != INVALID_ENTITY) isRoot = false;
            }
            if (isRoot) {
                Transform& t = scene.GetComponent<Transform>(e);
                // updated as root
                t.UpdateWorldAsRoot();

                // children
                if (scene.HasComponent<HierarchyComponent>(e)) {
                    HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(e);
                    Entity child = h.firstChild;
                    while (child != INVALID_ENTITY) {
                        UpdateRecursive(scene, child, t.model);
                        HierarchyComponent& childH = scene.GetComponent<HierarchyComponent>(child);
                        child = childH.nextSibling;
                    }
                }
            }
        }
    }

    // Atualiza somente a subtree de uma entidade (chame após edição no inspector ou gizmo)
    inline void UpdateSubtree(SceneECS& scene, Entity root) {
        // pega matrix do parent se existir
        glm::mat4 parentWorld = glm::mat4(1.0f);
        if (scene.HasComponent<HierarchyComponent>(root)) {
            HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(root);
            if (h.parent != INVALID_ENTITY && scene.HasComponent<Transform>(h.parent)) {
                parentWorld = scene.GetComponent<Transform>(h.parent).model;
            }
        }
        UpdateRecursive(scene, root, parentWorld);
    }
}


// Sistema ECS que roda no começo do frame
class TransformSystem : public System {
public:
    void Update(float deltaTime) override {
        TransformSys::UpdateAllTransforms(*GEngine->scene);
    }
};