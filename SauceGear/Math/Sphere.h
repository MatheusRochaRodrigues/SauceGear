#pragma once 
#include <glm/glm.hpp>
#include <cmath>
#include "Ray.h"

struct Sphere {
    glm::vec3 center;
    float radius;

    bool intersect(const Ray& ray, float& t) const {
        glm::vec3 oc = ray.origin - center;
        float b = glm::dot(oc, ray.direction);
        float c = glm::dot(oc, oc) - radius * radius;
        float h = b * b - c;

        if (h < 0.0f) return false;
        h = sqrt(h);
        t = -b - h;
        return t >= 0.0f;
    }
};
