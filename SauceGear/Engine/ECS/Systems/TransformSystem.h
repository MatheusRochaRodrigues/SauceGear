#pragma once
#include "../System.h"
#include "../../Core/EngineContext.h"
#include "../../Scene/SceneECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/HierarchyComponent.h"
#include <vector> 

// Helper simples para atualizar a árvore de transforms.
// Adapte scene.GetAllEntities() para a sua API de ECS se necessário.

namespace TransformSys {  
    // Atualiza uma entidade (já sabendo parentWorld) 
    inline void UpdateNode(SceneECS& scene,
        Entity e,
        const glm::mat4& parentWorld)
    {
        TransformComponent& t = scene.GetComponent<TransformComponent>(e);

        // Se nada sujo, não faz nada
        if (!t.localDirty && !t.worldDirty)
            return;

        // Atualiza world
        t.UpdateWorldFromParent(parentWorld);

        // Propaga para filhos
        if (!scene.HasComponent<HierarchyComponent>(e))
            return;

        HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(e);
        Entity child = h.firstChild;

        while (child != INVALID_ENTITY) {
            TransformComponent& ct = scene.GetComponent<TransformComponent>(child);
            ct.MarkWorldDirty(); // filhos precisam recalcular
            UpdateNode(scene, child, t.model);
            child = scene.GetComponent<HierarchyComponent>(child).nextSibling;
        }
    }
     
    // Atualiza como ROOT 
    inline void UpdateRoot(SceneECS& scene, Entity e)
    {
        TransformComponent& t = scene.GetComponent<TransformComponent>(e);

        if (!t.localDirty && !t.worldDirty)
            return;

        t.UpdateWorldAsRoot();

        if (!scene.HasComponent<HierarchyComponent>(e))
            return;

        HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(e);
        Entity child = h.firstChild;

        while (child != INVALID_ENTITY) {
            TransformComponent& ct = scene.GetComponent<TransformComponent>(child);
            ct.MarkWorldDirty();
            UpdateNode(scene, child, t.model);
            child = scene.GetComponent<HierarchyComponent>(child).nextSibling;
        }
    }
     
    // UPDATE GLOBAL (1x por frame) 
    inline void UpdateAll(SceneECS& scene)
    {
        const auto& entities = scene.GetAllEntities();

        for (Entity e : entities) {
            if (!scene.HasComponent<TransformComponent>(e))
                continue;

            bool isRoot = true;

            if (scene.HasComponent<HierarchyComponent>(e)) {
                HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(e);
                if (h.parent != INVALID_ENTITY)
                    isRoot = false;
            }

            if (!isRoot)
                continue;

            UpdateRoot(scene, e);
        }
    }

    // ---------------------------------------------
    // UPDATE SUBTREE (usado pelo Gizmo)
    // ---------------------------------------------
    inline void UpdateSubtree(SceneECS& scene, Entity root)
    {
        if (!scene.HasComponent<TransformComponent>(root))
            return;

        glm::mat4 parentWorld(1.0f);

        if (scene.HasComponent<HierarchyComponent>(root)) {
            HierarchyComponent& h = scene.GetComponent<HierarchyComponent>(root);
            if (h.parent != INVALID_ENTITY &&
                scene.HasComponent<TransformComponent>(h.parent)) {
                parentWorld = scene.GetComponent<TransformComponent>(h.parent).model;
            }
        }

        UpdateNode(scene, root, parentWorld);
    }
}


// Sistema ECS que roda no começo do frame
class TransformSystem : public System {
public:
    bool HasDirty(SceneECS& scene) {
        //preste atencao no const
        const auto& entities = scene.GetAllEntities();
        for (Entity e : entities) {
            if (!scene.HasComponent<TransformComponent>(e)) continue;
            TransformComponent& t = scene.GetComponent<TransformComponent>(e);
            if (t.localDirty || t.worldDirty) return true;
        }
        return false;
    }

    void Update(float deltaTime) override {
        try {
            // roda só se existir algum dirty
            if (!HasDirty(*GEngine->scene)) return; 
            TransformSys::UpdateAll(*GEngine->scene);

        } catch (const std::exception& e) {
            std::cerr << "[EXCEÇÃO - TransformSystem] " << e.what() << "\n";
        }
    }
};