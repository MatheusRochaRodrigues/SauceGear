#pragma once
#include "Components/AABBComponent.h"
#include "../Graphics/Mesh.h"

class utilsAABB {
public:
    static AABBComponent CalculateAABB(Mesh* mesh, const glm::mat4& transform) {
        AABBComponent aabb;
        aabb.min = glm::vec3(FLT_MAX);
        aabb.max = glm::vec3(-FLT_MAX);

        for (const auto& v : mesh->vertices) {
            glm::vec3 pos = glm::vec3(transform * glm::vec4(v.Position, 1.0f));
            aabb.min = glm::min(aabb.min, pos);
            aabb.max = glm::max(aabb.max, pos);
        }

        return aabb;
    }

    

};










//static BoundingSphereComponent CalculateBoundingSphere(Mesh* mesh, const glm::mat4& transform) {
    //    BoundingSphereComponent sphere;
    //    glm::vec3 centerSum(0.0f);
    //    int vertexCount = mesh->vertices.size();

    //    // Calcula centro médio
    //    for (const auto& v : mesh->vertices) {
    //        glm::vec3 pos = glm::vec3(transform * glm::vec4(v.Position, 1.0f));
    //        centerSum += pos;
    //    }
    //    sphere.center = centerSum / (float)vertexCount;

    //    // Calcula raio máximo
    //    float maxDist2 = 0.0f;
    //    for (const auto& v : mesh->vertices) {
    //        glm::vec3 pos = glm::vec3(transform * glm::vec4(v.Position, 1.0f));
    //        float dist2 = glm::length2(pos - sphere.center);
    //        maxDist2 = std::max(maxDist2, dist2);
    //    }
    //    sphere.radius = sqrt(maxDist2);

    //    return sphere;
    //}