#pragma once
#include <glm/glm.hpp>
#include <limits>

struct Bounds {
    glm::vec3 min;  //minCorner
    glm::vec3 max;  //maxCorner

    // -------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------
    Bounds()
        : min(std::numeric_limits<float>::max()),
        max(std::numeric_limits<float>::lowest()) {
    }

    Bounds(const glm::vec3& mn, const glm::vec3& mx)
    {
        min = glm::min(mn, mx);
        max = glm::max(mn, mx);
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

    inline bool contains_point(const glm::vec3& p) const {
        return (p.x >= min.x && p.x <= max.x &&
                p.y >= min.y && p.y <= max.y &&
                p.z >= min.z && p.z <= max.z);
    }

    // -------------------------------------------------------------
    // Intersection / union
    // -------------------------------------------------------------
    inline bool intersects(const Bounds& other) const {
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

    inline bool encloses(const Bounds& other) const {
        return (min.x <= other.min.x && max.x >= other.max.x &&
            min.y <= other.min.y && max.y >= other.max.y &&
            min.z <= other.min.z && max.z >= other.max.z);
    }

    inline Bounds intersected(const Bounds& other) const {
        glm::vec3 new_min = glm::max(min, other.min);
        glm::vec3 new_max = glm::min(max, other.max);

        if (new_min.x > new_max.x ||
            new_min.y > new_max.y ||
            new_min.z > new_max.z)
        {
            return Bounds(); // empty
        }
        return Bounds(new_min, new_max);
    }

    inline Bounds joined(const Bounds& other) const {
        return Bounds(
            glm::min(min, other.min),
            glm::max(max, other.max)
        );
    }

    // -------------------------------------------------------------
    // Expand / contract
    // -------------------------------------------------------------
    inline Bounds expanded(const glm::vec3& amt) const {
        return Bounds(min - amt, max + amt);
    }

    inline Bounds expanded(float amt) const {
        return expanded(glm::vec3(amt));
    }

    // -------------------------------------------------------------
    // Arithmetic operators
    // -------------------------------------------------------------
    inline Bounds operator*(float s) const {
        return Bounds(min * s, max * s);
    }

    inline Bounds operator*(const glm::vec3& v) const {
        return Bounds(min * v, max * v);
    }

    inline Bounds operator+(const glm::vec3& off) const {
        return Bounds(min + off, max + off);
    }
};
