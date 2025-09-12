#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <iostream>
#include "../Reflection/Macros.h"

// NOTE: Este Transform mantém:
//  - localPosition/localRotation/localScale  <- os dados editáveis (Inspector/Gizmo quando convertido)
//  - position/rotation/scale                <- caches world (usados pelo renderer e GetMatrix())
//  - model                                  <- worldMatrix final
//  - flags dirty para evitar recomputos desnecessários

struct Transform {
    // world (cache) - mantido por UpdateAllTransforms ou após decomposição
    glm::vec3 position = glm::vec3(0.0f);   // world position
    glm::quat rotation = glm::quat(glm::vec3(0.0f)); // world rotation (quat)
    glm::vec3 scale = glm::vec3(1.0f);   // world scale

    // local (editáveis)
    glm::vec3 localPosition = glm::vec3(0.0f);
    glm::quat localRotation = glm::quat(glm::vec3(0.0f));
    glm::vec3 localScale = glm::vec3(1.0f);

    // matrices/caches
    glm::mat4 localMatrix = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f); // = worldMatrix

    // flags
    bool localDirty = true;  // local changed (user edited)
    bool worldDirty = true;  // world/model dirty (need recompute)

    REFLECT_CLASS(Transform) {
        REFLECT_HEADER("Global Transform (read-only)");
        REFLECT_FIELD(position);
        REFLECT_FIELD(rotation);  // this is a quaternion (Inspector drawer will show as Euler)
        REFLECT_FIELD(scale);

        REFLECT_SPACE();

        REFLECT_HEADER("Local Transform (editable)");
        REFLECT_FIELD(localPosition);
        REFLECT_FIELD(localRotation);
        REFLECT_FIELD(localScale);

        REFLECT_ON_EDITED(
            // sempre que o inspector editar algo, marcamos local sujo
            obj->MarkLocalDirty();
        // opcional debug
        // std::cout << "Transform REFLECT_ON_EDITED fired\n";
        );
    }

    Transform() = default;

    // Recalcula localMatrix quando localDirty
    void UpdateLocalMatrixIfNeeded() {
        if (!localDirty) return;
        localMatrix = glm::translate(glm::mat4(1.0f), localPosition)
            * glm::toMat4(localRotation)
            * glm::scale(glm::mat4(1.0f), localScale);
        localDirty = false;
        MarkWorldDirty();
    }

    // Recalcula world matrix a partir do parentWorld (chame para cada entidade na ordem pai->filho)
    void UpdateWorldFromParent(const glm::mat4& parentWorld) {
        UpdateLocalMatrixIfNeeded();
        model = parentWorld * localMatrix;

        // Decompose model -> position/rotation/scale (world)
        DecomposeMatrix(model, position, rotation, scale);

        worldDirty = false;
    }

    // Recalcula como root (sem parent)
    void UpdateWorldAsRoot() {
        UpdateLocalMatrixIfNeeded();
        model = localMatrix;
        DecomposeMatrix(model, position, rotation, scale);
        worldDirty = false;
    }

    // GetMatrix retorna a matriz world (use UpdateAllTransforms antes de confiar)
    const glm::mat4& GetMatrix() {
        // assume UpdateAllTransforms foi chamado de forma a manter model atualizado.
        if (worldDirty) {
            // fallback simples: recompute from world position/rotation/scale
            model = glm::translate(glm::mat4(1.0f), position)
                * glm::toMat4(rotation)
                * glm::scale(glm::mat4(1.0f), scale);
            worldDirty = false;
        }
        return model;
    }

    // Marca local como alterado (ex.: inspector)
    void MarkLocalDirty() {
        localDirty = true;
        MarkWorldDirty(); // se local sujou, mundo também
    }

    // marca world dirty (filhos precisam recalcular)
    void MarkWorldDirty() {
        worldDirty = true;
        // filhos serão sujos no UpdateAllTransforms
    }

    // Helper: decompose uma matrix para (pos, quat, scale)
    static bool DecomposeMatrix(const glm::mat4& m, glm::vec3& outPos, glm::quat& outRot, glm::vec3& outScale) {
        using namespace glm;
        vec3 skew;
        vec4 perspective;
        if (!decompose(m, outScale, outRot, outPos, skew, perspective)) return false;
        outRot = glm::normalize(outRot);
        return true;
    }

    // Quando ImGuizmo retorna uma nova world matrix (newWorld):
    // - Se existe parentWorld, local = inverse(parentWorld) * newWorld -> decompose local e salvar em local*
    // - Se nao tem parent -> decompose newWorld em local* (comportamento de "mover root")
    bool SetLocalFromWorldMatrix(const glm::mat4& newWorld, const glm::mat4& parentWorld) {
        glm::mat4 invParent = glm::inverse(parentWorld);
        glm::mat4 newLocal = invParent * newWorld;

        glm::vec3 lp; glm::quat lr; glm::vec3 ls;
        if (!DecomposeMatrix(newLocal, lp, lr, ls)) return false;

        localPosition = lp;
        localRotation = glm::normalize(lr);
        localScale = ls;
        MarkLocalDirty();
        return true;
    }

    bool SetLocalFromWorldMatrixAsRoot(const glm::mat4& newWorld) {
        glm::vec3 lp; glm::quat lr; glm::vec3 ls;
        if (!DecomposeMatrix(newWorld, lp, lr, ls)) return false;

        localPosition = lp;
        localRotation = glm::normalize(lr);
        localScale = ls;
        MarkLocalDirty();
        return true;
    }

    // Caso alguém queira decompor worldMatrix diretamente para world fields (compat)
    bool DecomposeTransformToWorld(const glm::mat4& transformMatrix) {
        using namespace glm;
        vec3 skew;
        vec4 perspective;
        quat orientation;
        bool success = decompose(transformMatrix, scale, orientation, position, skew, perspective);
        if (!success) return false;
        rotation = glm::normalize(orientation);
        MarkWorldDirty();
        return true;
    }


    glm::vec3 GetForwardDirection() const {
        return glm::normalize(rotation * glm::vec3(0, 0, -1));
    }
};
