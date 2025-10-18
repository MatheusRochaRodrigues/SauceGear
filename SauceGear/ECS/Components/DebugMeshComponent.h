#pragma once 
#include <glm/glm.hpp>"

struct DebugMeshComponent {
    //Mesh* mesh = nullptr;
    glm::mat4 transform = glm::mat4(1.0f);
    glm::vec3 color = glm::vec3(1.0f);
    bool showWireframe = false;
    bool showNormals = false;
};