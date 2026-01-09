#pragma once 
#include <glm/glm.hpp>"
#include "../Reflection/Macros.h"

struct DebugMeshComponent {
    //Mesh* mesh = nullptr;
    glm::mat4 transform = glm::mat4(1.0f);
    glm::vec3 color = glm::vec3(1.0f);
    bool showWireframe = false;
    bool showNormals = false;
    bool showBox = false;
    glm::vec3 colorBox = glm::vec3(1.0f, 0, 1.0f);


    REFLECT_CLASS(DebugMeshComponent) {
        REFLECT_HEADER("DebugMesh");
        REFLECT_FIELD(color);
        REFLECT_FIELD(showWireframe);

        REFLECT_HEADER("AABB");
        REFLECT_FIELD(showBox);
        REFLECT_FIELD(colorBox);
    }
};