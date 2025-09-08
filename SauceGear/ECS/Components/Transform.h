#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../Reflection/Macros.h"

struct Transform {
    glm::vec3 position = glm::vec3(0.0f);
    glm::quat rotation = glm::quat(glm::vec3(0.0f)); // agora em quaternion
    glm::vec3 scale = glm::vec3(1.0f);

    glm::vec3 localPosition = glm::vec3(0.0f);
    glm::quat localRotation = glm::quat(glm::vec3(0.0f));
    glm::vec3 localScale = glm::vec3(1.0f);

    glm::mat4 model;
    bool dirty = true;

    REFLECT_CLASS(Transform) {
        REFLECT_HEADER("Global Transform");
        REFLECT_FIELD(position);
        REFLECT_FIELD(rotation);  // Inspector trata especial (quat ↔ Euler)
        REFLECT_FIELD(scale);

        REFLECT_SPACE();

        REFLECT_HEADER("Local Transform")
            REFLECT_FIELD(localPosition);
        REFLECT_FIELD(localRotation);
        REFLECT_FIELD(localScale);

        REFLECT_ON_EDITED(
            obj->MarkDirty();
            std::cout << "alterado" << std::endl;
        );

    }

    Transform() = default;

    Transform(glm::vec3 pos, glm::quat rot = glm::quat(glm::vec3(0.0f)), glm::vec3 scl = glm::vec3(1.0f))
        : position(pos), rotation(rot), scale(scl) {
    }

    const glm::mat4& GetMatrix() {
        if (dirty) {
            model = glm::translate(glm::mat4(1.0f), position)
                * glm::toMat4(rotation)
                * glm::scale(glm::mat4(1.0f), scale);
            dirty = false;
        }
        return model;
    }

    glm::vec3 GetForwardDirection() const {
        return glm::normalize(rotation * glm::vec3(0, 0, -1));
    }

    bool DecomposeTransform(const glm::mat4& transformMatrix) {
        using namespace glm;
        vec3 skew;
        vec4 perspective;
        quat orientation;

        bool success = decompose(transformMatrix, scale, orientation, position, skew, perspective);
        if (!success) return false;

        rotation = glm::normalize(orientation);
        return true;
    }

    void MarkDirty() { dirty = true; }
};
