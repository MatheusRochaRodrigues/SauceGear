#pragma once
#include <glm/glm.hpp>
#include "Ray.h"

struct Plane {
    glm::vec3 normal;
    float d; // ax + by + cz + d = 0

    bool intersect(const Ray& ray, float& t) const {
        float denom = glm::dot(normal, ray.direction);
        if (fabs(denom) < 1e-6f) return false;
        t = -(glm::dot(normal, ray.origin) + d) / denom;
        return t >= 0.0f;
    }
};
