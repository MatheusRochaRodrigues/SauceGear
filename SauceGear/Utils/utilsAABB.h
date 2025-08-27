#pragma once
#include "../Scene/Components/AABBComponent.h"
#include "../Graphics/Mesh.h"

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction; // normalizado
};

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

    //Isso pega a posição do mouse, transforma de NDC para world space, e retorna um raio.
    static Ray ScreenPosToWorldRay(
        float mouseX, float mouseY,
        float screenWidth, float screenHeight,
        const glm::mat4& view, const glm::mat4& projection,
        const glm::vec3& cameraPos)
    {
        // NDC
        float x = (2.0f * mouseX) / screenWidth - 1.0f;
        float y = 1.0f - (2.0f * mouseY) / screenHeight;

        glm::vec4 nearPointNDC(x, y, -1.0f, 1.0f);
        glm::vec4 farPointNDC(x, y, 1.0f, 1.0f);

        glm::mat4 invVP = glm::inverse(projection * view);

        glm::vec4 nearWorld = invVP * nearPointNDC; nearWorld /= nearWorld.w;
        glm::vec4 farWorld = invVP * farPointNDC;  farWorld /= farWorld.w;

        Ray ray;
        ray.origin = cameraPos;
        ray.direction = glm::normalize(glm::vec3(farWorld - nearWorld));
        return ray;
    }


    static bool IntersectRayAABB(const Ray& ray, const AABBComponent& aabb, float& tNear) {
        tNear = 0.0f;
        float tFar = FLT_MAX;

        for (int i = 0; i < 3; ++i) {
            if (fabs(ray.direction[i]) < 1e-6f) {
                if (ray.origin[i] < aabb.min[i] || ray.origin[i] > aabb.max[i])
                    return false;
            }
            else {
                float t1 = (aabb.min[i] - ray.origin[i]) / ray.direction[i];
                float t2 = (aabb.max[i] - ray.origin[i]) / ray.direction[i];
                if (t1 > t2) std::swap(t1, t2);
                tNear = std::max(tNear, t1);
                tFar = std::min(tFar, t2);
                if (tNear > tFar) return false;
            }
        }
        return true;
    }
    //Teste rápido para saber se o raio “atingiu” a entidade via AABB.
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