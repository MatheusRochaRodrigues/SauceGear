#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp> 
#include <glm/gtx/matrix_decompose.hpp>
#include "../Reflection/Macros.h"

struct TransformComponent {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f); // Euler
    glm::vec3 scale = glm::vec3(1.0f);

    glm::vec3 localPosition = glm::vec3(0.0f);
    glm::vec3 localRotation = glm::vec3(0.0f); // Euler
    glm::vec3 localScale = glm::vec3(1.0f);

    glm::mat4 model;
    bool dirty = true;

    /*REFLECT_CLASS(Transform) {
        REFLECT_FIELD(Transform, position);
        REFLECT_FIELD(Transform, rotation);
        REFLECT_FIELD(Transform, scale);
    }*/

    REFLECT_CLASS(TransformComponent) {
        REFLECT_HEADER("Global Transform");
        REFLECT_FIELD(position);
        REFLECT_FIELD(rotation);
        REFLECT_FIELD(scale);

        REFLECT_SPACE();

        REFLECT_HEADER("Local Transform")
            REFLECT_FIELD(localPosition);
        REFLECT_FIELD(localRotation);
        REFLECT_FIELD(localScale);
    }

    TransformComponent() = default;

    TransformComponent(glm::vec3 pos, glm::vec3 rot = glm::vec3(0.0f), glm::vec3 scl = glm::vec3(1.0f))
        : position(pos), rotation(rot), scale(scl) {
    }


    /*glm::mat4 GetMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, scale);

        return model;
    }*/

    const glm::mat4& GetMatrix() {
        if (dirty) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
            model = glm::scale(model, scale);
            dirty = true;
            // pode chamar o calculo do bolding box pro picking
        }
        return model;
    }

    glm::vec3 GetForwardDirection(const glm::vec3& rotationEuler) {
        glm::vec3 direction;
        //mais segguro de aplicar rotações yawPitchRoll dq rotate separada para cada eixo
        glm::mat4 rotationMatrix = glm::yawPitchRoll(glm::radians(rotationEuler.y),
            glm::radians(rotationEuler.x),
            glm::radians(rotationEuler.z));
        direction = glm::vec3(rotationMatrix * glm::vec4(0, 0, -1, 0)); // forward

        return glm::normalize(direction);
    }

    glm::vec3 GetForwardDirection() {
        glm::vec3& rotationEuler = rotation;
        glm::vec3 direction;
        //mais segguro de aplicar rotações yawPitchRoll dq rotate separada para cada eixo
        glm::mat4 rotationMatrix = glm::yawPitchRoll(glm::radians(rotationEuler.y),
            glm::radians(rotationEuler.x),
            glm::radians(rotationEuler.z));
        direction = glm::vec3(rotationMatrix * glm::vec4(0, 0, -1, 0)); // forward

        return glm::normalize(direction);
    }

    bool DecomposeTransform(const glm::mat4& transformMatrix) {
        using namespace glm;
        vec3 skew;
        vec4 perspective;
        quat orientation;

        bool success = decompose(transformMatrix, scale, orientation, position, skew, perspective);
        if (!success) return false;

        // Converter quat para Euler em graus, com consideração da ordem de rotação
        rotation = degrees(eulerAngles(orientation));
        // Optional: Restrict range to prevent issues with gimbal lock
        rotation.y = glm::mod(rotation.y, 360.0f);  // Garantir que o eixo Y permaneça dentro do intervalo 0-360
        return true;
    }

    /*bool DecomposeTransform(const glm::mat4& matrix) {
        using namespace glm;
        vec3 skew;
        vec4 perspective;

        if (!decompose(matrix, scale, rotation, position, skew, perspective))
            return false;

        return true;
    }*/

    void MarkDirty() { dirty = true; }

};


