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
        if (t.localDirty || t.worldDirty)          // Só recalcula se estiver marcado como sujo
            t.UpdateWorldFromParent(parentWorld);

        // FAZ RECURSÃO para filhos e Propaga "dirty" para os filhos
        if (scene.HasComponent<HierarchyComponent>(entity)) {
            HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(entity);
            Entity child = h.firstChild;
            while (child != INVALID_ENTITY) {
                Transform& ct = scene.GetComponent<Transform>(child);
                ct.MarkWorldDirty(); // marca os filhos como sujos

                UpdateRecursive(scene, child, t.model); 
                child = scene.GetComponent<HierarchyComponent>(child).nextSibling;
            }
        }
    }

    // Atualiza todas entidades: encontra raízes (parent == INVALID_ENTITY) e atualiza a árvore
    inline void UpdateAllTransforms(SceneECS& scene) {
        // --- NOTE: adapte esta iteração à sua API ---
        std::vector<Entity> all = scene.GetAllEntities(); // se você não tiver, troque por seu método
        for (Entity e : all) {
            if (!scene.HasComponent<Transform>(e)) continue;
             
            // só processa se estiver sujo
            Transform& t = scene.GetComponent<Transform>(e);
            if (!(t.localDirty || t.worldDirty)) continue;

            t.transformChangedThisFrame = true;

            // root detection: se tem HierarchyComponent e parent != INVALID_ENTITY -> não é root
            bool isRoot = true;
            if (scene.HasComponent<HierarchyComponent>(e)) {
                HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(e);
                if (h.parent != INVALID_ENTITY) isRoot = false;
            }
            if (isRoot) {
                // updated as root
                t.UpdateWorldAsRoot();

                // children                 // agora Propagamos dirty pros filhos
                if (scene.HasComponent<HierarchyComponent>(e)) {
                    HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(e);
                    Entity child = h.firstChild;
                    while (child != INVALID_ENTITY) {
                        Transform& ct = scene.GetComponent<Transform>(child);
                        ct.MarkWorldDirty();

                        UpdateRecursive(scene, child, t.model);
                        child = scene.GetComponent<HierarchyComponent>(child).nextSibling;
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



    //void MarkDirtyRecursive(SceneECS& scene, Entity self) {
    //    Transform& tc = scene.GetComponent<Transform>(self);
    //    if (tc.dirty) return; // já marcado

    //    dirty = true;

    //    // se tiver filhos, marca eles também
    //    if (scene.HasComponent<HierarchyComponent>(self)) {
    //        HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(self);
    //        Entity child = h.firstChild;
    //        while (child != INVALID_ENTITY) {
    //            if (scene.HasComponent<Transform>(child)) {
    //                scene.GetComponent<Transform>(child).MarkDirty(scene, child);
    //            }
    //            child = scene.GetComponent<HierarchyComponent>(child).nextSibling;
    //        }
    //    }
    //}

}


// Sistema ECS que roda no começo do frame
class TransformSystem : public System {
public:
    bool HasDirty(SceneECS& scene) {
        //preste atencao no const
        const auto& entities = scene.GetAllEntities();
        for (Entity e : entities) {
            if (!scene.HasComponent<Transform>(e)) continue;
            Transform& t = scene.GetComponent<Transform>(e);
            if (t.localDirty || t.worldDirty) return true;
        }
        return false;
    }

    void Update(float deltaTime) override {
        try {
            // roda só se existir algum dirty
            if (!HasDirty(*GEngine->scene)) return; 
            TransformSys::UpdateAllTransforms(*GEngine->scene);

        } catch (const std::exception& e) {
            std::cerr << "[EXCEÇÃO - DebugRenderer] " << e.what() << "\n";
        }
    }
};