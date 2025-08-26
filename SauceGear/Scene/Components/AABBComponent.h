#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <glm/gtx/matrix_decompose.hpp>

struct AABBComponent {
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);
};

//struct BoundingSphereComponent {
//    glm::vec3 center = glm::vec3(0.0f);
//    float radius = 0.0f;
//};

