#pragma once
#include "SignedDistanceField.h"
#include "../../../../../Utils/PerlinNoise.h" 

using namespace glm;

class Terrain : public SignedDistanceField {
public:
    PerlinNoise noise;
    // ======== Altura do terreno ========
    float get_heightmap(vec2 pos) const {
        float base = noise.fbm(pos * 0.01f) * 20.0f;  // montanhas
        float detail = noise.fbm(pos * 0.1f) * 2.0f;  // detalhes
        return base + detail;
    } 

    // ======== SDF ========
    float sdfDistance(const vec3& p) const override {
        //float height = get_heightmap(vec2(p.x, p.z));
        //return p.y - height;
        //return p.y + 5.0f;
        return p.y + noise.fbm(p * 0.01f) * 5.0f;
    }

    //cavernas em terreno normal
    /*
    float sdfDistance(vec3 p) {
        float terrain = p.y - get_heightmap(vec2(p.x, p.z));
        float caves = noise.noise(p * 0.05f) - 0.2f;
        return max(terrain, -caves);
    }
    */
};

    