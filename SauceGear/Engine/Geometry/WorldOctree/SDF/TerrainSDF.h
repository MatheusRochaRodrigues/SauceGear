#pragma once
#include "SignedDistanceField.h"
#include "../../../Utils/Noise.h" 

using namespace glm;

class TerrainSDF : public SignedDistanceField {
public:

    // ======== Altura do terreno ========
    float sample_height(vec2 pos) const {
        float base = fbm(pos * 0.01f) * 20.0f;  // montanhas
        float detail = fbm(pos * 0.1f) * 2.0f;  // detalhes
        return base + detail;
    }

    // ======== Gradiente aproximado (derivada numérica) ========
    vec2 sample_gradient(vec2 pos) const {
        const float h = 0.1f;

        float hx1 = sample_height(pos + vec2(h, 0));
        float hx2 = sample_height(pos - vec2(h, 0));

        float hy1 = sample_height(pos + vec2(0, h));
        float hy2 = sample_height(pos - vec2(0, h));

        return vec2(hx1 - hx2, hy1 - hy2);
    }

    // ======== SDF ========
    float sdfDistance(const vec3& pos) const override {
        // altura do terreno naquela posição XZ
        float height = sample_height(vec2(pos.x, pos.z));

        // inclinação do terreno (gradiente do noise)
        vec2 grad = sample_gradient(vec2(pos.x, pos.z));

        // normal aproximada
        vec3 normal = normalize(vec3(grad.x, 1.0f, grad.y));

        // distância assinada
        return (pos.y - height) / length(normal);
    }
};

