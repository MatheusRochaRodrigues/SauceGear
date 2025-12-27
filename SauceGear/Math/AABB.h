#pragma once  
#include <limits>
#include <vector>
#include "../Math/Ray.h"

struct AABB {                             //Bounds(generic) Type AABB
    glm::vec3 min;  //minCorner
    glm::vec3 max;  //maxCorner

    // -------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------
    AABB()
        : min(std::numeric_limits<float>::max()),
        max(std::numeric_limits<float>::lowest()) {
    }

    AABB(const glm::vec3& mn, const glm::vec3& mx)
    {
        min = glm::min(mn, mx);
        max = glm::max(mn, mx);
    }



    // -------------------------------------------------------------
    // Helpers
    // ------------------------------------------------------------- 
    glm::vec3 corner(int i) const {
        return glm::vec3(
            (i & 1) ? max.x : min.x,
            (i & 2) ? max.y : min.y,
            (i & 4) ? max.z : min.z
        );
    }

    // -------------------------------------------------------------
    // Basic getters
    // -------------------------------------------------------------
    inline bool is_valid() const {
        return (min.x <= max.x && min.y <= max.y && min.z <= max.z);
    }

    inline glm::vec3 center() const {
        return (min + max) * 0.5f;
    }

    inline glm::vec3 size() const {
        return max - min;
    }

    inline bool contains(const glm::vec3& p) const {    // point
        return (p.x >= min.x && p.x <= max.x &&
                p.y >= min.y && p.y <= max.y &&
                p.z >= min.z && p.z <= max.z);
    }

    // -------------------------------------------------------------
    // Intersection / union
    // -------------------------------------------------------------
    inline bool intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x &&
                min.y <= other.max.y && max.y >= other.min.y &&
                min.z <= other.max.z && max.z >= other.min.z);
    }

    inline bool intersects(const glm::vec3 min, const glm::vec3 max, const glm::vec3 otherMin, const glm::vec3 otherMax) const {
        // Separating Axis Theorem for AABB
        if (max.x < otherMin.x || min.x > otherMax.x) return false;
        if (max.y < otherMin.y || min.y > otherMax.y) return false;
        if (max.z < otherMin.z || min.z > otherMax.z) return false;
        return true;
    }

    inline bool intersects(const glm::vec3 otherMin, const glm::vec3 otherMax) const {
        // Separating Axis Theorem for AABB
        if (max.x < otherMin.x || min.x > otherMax.x) return false;
        if (max.y < otherMin.y || min.y > otherMax.y) return false;
        if (max.z < otherMin.z || min.z > otherMax.z) return false;
        return true;
    }

     
    //Teste rápido para saber se o raio “atingiu” a entidade via AABB.
    bool intersects(const Ray& ray, float& tNear) const {
        float tFar = FLT_MAX;
        tNear = 0.0f;

        for (int i = 0; i < 3; ++i) {   //for each axis
            if (fabs(ray.direction[i]) < 1e-6f) {
                if (ray.origin[i] < min[i] || ray.origin[i] > max[i]) return false;
            }
            else {
                float t1 = (min[i] - ray.origin[i]) / ray.direction[i];
                float t2 = (max[i] - ray.origin[i]) / ray.direction[i];
                if (t1 > t2) std::swap(t1, t2);
                tNear = std::max(tNear, t1);
                tFar = std::min(tFar, t2);
                if (tNear > tFar) return false;
            }
        }
        return true;
    }

    /*bool Intersects(const Ray& ray, float& tNear) const {
        float tMin = 0.0f;
        float tMax = FLT_MAX;

        for (int i = 0; i < 3; i++) {
            if (fabs(ray.direction[i]) < 1e-6f) {
                if (ray.origin[i] < min[i] || ray.origin[i] > max[i])
                    return false;
            }
            else {
                float ood = 1.0f / ray.direction[i];
                float t1 = (min[i] - ray.origin[i]) * ood;
                float t2 = (max[i] - ray.origin[i]) * ood;
                if (t1 > t2) std::swap(t1, t2);
                tMin = std::max(tMin, t1);
                tMax = std::min(tMax, t2);
                if (tMin > tMax) return false;
            }
        } 
        tNear = tMin;
        return true;
    }*/


    inline bool encloses(const AABB& other) const {
        return (min.x <= other.min.x && max.x >= other.max.x &&
            min.y <= other.min.y && max.y >= other.max.y &&
            min.z <= other.min.z && max.z >= other.max.z);
    }

    inline AABB intersected(const AABB& other) const {
        glm::vec3 new_min = glm::max(min, other.min);
        glm::vec3 new_max = glm::min(max, other.max);

        if (new_min.x > new_max.x ||
            new_min.y > new_max.y ||
            new_min.z > new_max.z)
        {
            return AABB(); // empty
        }
        return AABB(new_min, new_max);
    }

    inline AABB joined(const AABB& other) const {
        return AABB(
            glm::min(min, other.min),
            glm::max(max, other.max)
        );
    }

    // -------------------------------------------------------------
    // Expand / contract
    // -------------------------------------------------------------
    inline AABB expanded(const glm::vec3& amt) const {
        return AABB(min - amt, max + amt);
    }

    inline AABB expanded(float amt) const {
        return expanded(glm::vec3(amt));
    }

    // -------------------------------------------------------------
    // Arithmetic operators
    // -------------------------------------------------------------
    inline AABB operator*(float s) const {
        return AABB(min * s, max * s);
    }

    inline AABB operator*(const glm::vec3& v) const {
        return AABB(min * v, max * v);
    }

    inline AABB operator+(const glm::vec3& off) const {
        return AABB(min + off, max + off);
    }
};
