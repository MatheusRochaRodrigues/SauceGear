#pragma once
#include "SignedDistanceField.h"
#include "../../../Utils/PerlinNoise.h"

class BasicTerrainSDF : public SignedDistanceField {
public:

    float sample_height(glm::vec2 pos) const {
        static PerlinNoise perlin(1337);
        float n = perlin.noise(pos * 0.01f) * 20.0f;
        return n;
    }

    glm::vec2 sample_gradient(glm::vec2 pos) const {
        const float h = 0.1f;
        float hx1 = sample_height(pos + glm::vec2(h, 0));
        float hx2 = sample_height(pos - glm::vec2(h, 0));
        float hy1 = sample_height(pos + glm::vec2(0, h));
        float hy2 = sample_height(pos - glm::vec2(0, h));
        return glm::vec2(hx1 - hx2, hy1 - hy2);
    }

    float sdfDistance(const glm::vec3& pos) const override {
        float height = sample_height(glm::vec2(pos.x, pos.z));
        glm::vec2 grad = sample_gradient(glm::vec2(pos.x, pos.z));
        glm::vec3 normal = glm::normalize(glm::vec3(grad.x, 1.0f, grad.y));
        return (pos.y - height) / glm::length(normal);
    }
};
