#pragma once
#include <glm/glm.hpp>

using namespace glm;

static inline float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

static float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0f - 2.0f * f);

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

static float fbm(vec2 p) {
    float total = 0;
    float amp = 1.0;
    float freq = 1.0;

    for (int i = 0; i < 5; i++) {
        total += noise(p * freq) * amp;
        freq *= 2.0;
        amp *= 0.5f;
    }

    return total;
}
