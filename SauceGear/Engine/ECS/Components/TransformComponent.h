#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <iostream>
#include "../Reflection/Macros.h"

// NOTE:
// - local*  = SOURCE OF TRUTH (editável)
// - world*  = CACHE (somente leitura lógica)       // - NUNCA escrever world* diretamente
// - model   = world matrix final
 
struct TransformComponent { 
    // WORLD (CACHE) 
    glm::vec3 position{ 0.0f };         //readOnly
    glm::quat rotation{ 1, 0, 0, 0 };   //readOnly
    glm::vec3 scale{ 1.0f };            //readOnly

    glm::mat4 model{ 1.0f };
     
    // LOCAL (SOURCE OF TRUTH) 
    glm::vec3 localPosition{ 0.0f };
    glm::quat localRotation{ 1, 0, 0, 0 };
    glm::vec3 localScale{ 1.0f };

    glm::mat4 localMatrix{ 1.0f };
     
    // FLAGS 
    bool localDirty = true;
    bool worldDirty = true;
    bool transformChangedThisFrame = false;
     
    // REFLECTION 
    REFLECT_CLASS(TransformComponent) {
        REFLECT_HEADER("Global TransformComponent (read-only)");
        REFLECT_FIELD(position);
        REFLECT_FIELD(rotation);
        REFLECT_FIELD(scale);

        REFLECT_SPACE();

        REFLECT_HEADER("Local TransformComponent (editable)");
        REFLECT_FIELD(localPosition);
        REFLECT_FIELD(localRotation);
        REFLECT_FIELD(localScale);

        REFLECT_ON_EDITED(
            obj->MarkLocalDirty();
            obj->UpdateWorldAsRoot(); // ou via parent
        );
    }
     
    // LOCAL SETTERS 
    void SetLocalPosition(const glm::vec3& pos) {
        localPosition = pos;
        std::cout << "movido" << std::endl;
        MarkLocalDirty();
    }

    void SetLocalRotation(const glm::quat& rot) {
        localRotation = glm::normalize(rot);
        MarkLocalDirty();
    }

    void SetLocalScale(const glm::vec3& scl) {
        localScale = scl;
        MarkLocalDirty();
    }

    void SetLocalTRS(const glm::vec3& pos,
        const glm::quat& rot,
        const glm::vec3& scl) {
        localPosition = pos;
        localRotation = glm::normalize(rot);
        localScale = scl;
        MarkLocalDirty();
    }

    void SetLocalFromMatrix(const glm::mat4& mat) {
        glm::vec3 pos, scl;
        glm::quat rot;

        if (!DecomposeMatrix(mat, pos, rot, scl)) {
            std::cerr << "[TransformComponent] Failed to decompose local matrix\n";
            return;
        }

        localPosition = pos;
        localRotation = rot;
        localScale = scl;

        MarkLocalDirty();
    }

     
    // WORLD SETTERS (SAFE) 
    //  -NÃO escrevem cache     //  -Convertem world -> local 
    void SetWorldPosition(const glm::vec3& worldPos,
        const glm::mat4& parentWorld = glm::mat4(1.0f)) {
        glm::vec4 lp = glm::inverse(parentWorld) * glm::vec4(worldPos, 1.0f);
        localPosition = glm::vec3(lp);
        MarkLocalDirty();
    }

    void SetWorldRotation(const glm::quat& worldRot,
        const glm::mat4& parentWorld = glm::mat4(1.0f)) {
        glm::quat parentRot;
        glm::vec3 skew, pos, scl;
        glm::vec4 persp;

        glm::decompose(parentWorld, scl, parentRot, pos, skew, persp);
        parentRot = glm::normalize(parentRot);

        localRotation = glm::normalize(glm::inverse(parentRot) * worldRot);
        MarkLocalDirty();
    }

    void SetWorldScale(const glm::vec3& worldScale,
        const glm::mat4& parentWorld = glm::mat4(1.0f)) {
        glm::vec3 parentScale;
        glm::quat r;
        glm::vec3 p, skew;
        glm::vec4 persp;

        glm::decompose(parentWorld, parentScale, r, p, skew, persp);
        localScale = worldScale / parentScale;
        MarkLocalDirty();
    }
     

    //-----------------------------------------------
    //--------------- UPDATE CORE -------------------
    //-----------------------------------------------
    void UpdateLocalMatrixIfNeeded() {
        if (!localDirty) return;

        localMatrix =
            glm::translate(glm::mat4(1.0f), localPosition) *
            glm::toMat4(localRotation) *
            glm::scale(glm::mat4(1.0f), localScale);

        localDirty = false;
        worldDirty = true;
    }

    void UpdateWorldFromParent(const glm::mat4& parentWorld) {
        UpdateLocalMatrixIfNeeded();

        if (!worldDirty) return;

        model = parentWorld * localMatrix;
        DecomposeMatrix(model, position, rotation, scale);

        worldDirty = false;
        transformChangedThisFrame = true;
    }

    void UpdateWorldAsRoot() {
        UpdateLocalMatrixIfNeeded();

        if (!worldDirty) return;

        model = localMatrix;
        DecomposeMatrix(model, position, rotation, scale);

        worldDirty = false;
        transformChangedThisFrame = true;
    }
    //------------------------------------------------------ 


    // ACCESS 
    const glm::mat4& GetMatrix() const { return model; }
     
    // DIRTY CONTROL 
    void MarkLocalDirty() { 
        localDirty = true;      worldDirty = true; 
    }
    void MarkWorldDirty() { worldDirty = true;                      }
     
    // HELPERS 
    static bool DecomposeMatrix(const glm::mat4& mat,
        glm::vec3& pos,
        glm::quat& rot,
        glm::vec3& scl) {
        glm::vec3 skew;
        glm::vec4 perspective;
        if (!glm::decompose(mat, scl, rot, pos, skew, perspective))
            return false;
        rot = glm::normalize(rot);
        return true;
    }

    glm::vec3 GetForwardDirection() const {
        return glm::normalize(rotation * glm::vec3(0, 0, -1));
    }
};
