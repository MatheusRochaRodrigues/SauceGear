#pragma once
#include "SignedDistanceField.h"
#include "../../../../../Utils/PerlinNoise.h"  
using namespace glm;

class Planet : public SignedDistanceField {
public:
    vec3 center;
    float radius = 3;       //5
    Planet() { center = vec3(0); };

    // ======== SDF ========
    //planeta perfeitamente esférico
    /*
    float sdfDistance(const vec3& p) const override {
        vec3 center = center;
        float dist = length(p - center);
        return dist - radius;
    }
    */

    //Para deixar irregular, você soma ruído
    float sdfDistance(const vec3& p) const override {
        float base = length(p - center) - radius;

        glm::vec3 scaled = p * 0.01f;           // escala espacial do ruído
        float n = noise.fbm(scaled) * 10/*40.0*/;            // usa fbm(glm::vec3)        //opcional o  * 40.0
         

        return base + 0 /*0*/; //n
    }

    /*
    float cave_scale = 1;
    float cave_threshold = 1/2;
    float sdfDistance(vec3 p) {
        float planet = length(p) - radius;

        float caves = noise.noise(p * cave_scale) - cave_threshold;

        // Usando união/subtração via operações booleanas de SDF
        return max(planet, -caves); // coloca cavernas dentro do planeta
    }
    */

private:
    PerlinNoise noise;
}; 
