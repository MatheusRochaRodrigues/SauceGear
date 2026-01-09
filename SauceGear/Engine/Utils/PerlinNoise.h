#pragma once
#include <glm/glm.hpp>
#include <array>
#include <random>

class PerlinNoise {
public:
    PerlinNoise(unsigned int seed = 2025);

    // ----------------------
    // Noise Perlin padrão
    // ----------------------
    float noise(float x, float y) const;
    float noise(float x, float y, float z) const;

    float noise(const glm::vec2& p) const { return noise(p.x, p.y); }
    float noise(const glm::vec3& p) const { return noise(p.x, p.y, p.z); }

    // ----------------------
    // fBm - Fractional Brownian Motion
    // ----------------------
    float fbm(const glm::vec2& p, int octaves = 6, float lacunarity = 2.0f, float gain = 0.5f) const;
    float fbm(const glm::vec3& p, int octaves = 6, float lacunarity = 2.0f, float gain = 0.5f) const;

    // ----------------------
    // Fractal Noise Variants
    // ----------------------
    float fractalBrownian(const glm::vec2& p, int octaves = 6) const { return fbm(p, octaves); }
    float fractalBrownian(const glm::vec3& p, int octaves = 6) const { return fbm(p, octaves); }

    // ----------------------
    // Ridged Noise (para montanhas)
    // ----------------------
    float ridged(const glm::vec2& p, int octaves = 6, float lacunarity = 2.0f, float gain = 0.5f) const;
    float ridged(const glm::vec3& p, int octaves = 6, float lacunarity = 2.0f, float gain = 0.5f) const;

private:
    std::array<int, 512> perm;

    // Helpers
    float fade(float t) const;
    float lerp(float a, float b, float t) const;

    float grad(int hash, float x, float y) const;
    float grad(int hash, float x, float y, float z) const;
};
