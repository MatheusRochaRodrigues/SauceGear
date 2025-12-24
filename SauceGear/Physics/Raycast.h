#pragma once 
#include <vector>
#include <cfloat>
#include "../Math/Ray.h"
#include "../Math/AABB.h"

struct RaycastHit {
    float t;
    glm::vec3 point;
    glm::vec3 normal;
    int entity;
};

inline bool Raycast(
    const Ray& ray,
    const std::vector<AABB>& aabbs,
    RaycastHit& hit)
{
    bool found = false;
    hit.t = FLT_MAX;

    for (size_t i = 0; i < aabbs.size(); ++i) {
        float t;
        if (aabbs[i].intersect(ray, t) && t < hit.t) {
            hit.t = t;
            hit.point = ray.at(t);
            hit.entity = (int)i;
            found = true;
        }
    }
    return found;
}
