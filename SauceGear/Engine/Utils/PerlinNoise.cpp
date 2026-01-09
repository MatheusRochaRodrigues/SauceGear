#include "PerlinNoise.h"

PerlinNoise::PerlinNoise(unsigned int seed) {
    std::array<int, 256> p;
    for (int i = 0; i < 256; i++)
        p[i] = i;

    std::default_random_engine eng(seed);
    std::shuffle(p.begin(), p.end(), eng);

    // Duplica
    for (int i = 0; i < 256; i++) {
        perm[i] = p[i];
        perm[256 + i] = p[i];
    }
}

float PerlinNoise::fade(float t) const {
    // Curva suave quintica
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::lerp(float a, float b, float t) const {
    return a + t * (b - a);
}

float PerlinNoise::grad(int hash, float x, float y) const {
    int h = hash & 3;
    float u = h < 2 ? x : y;
    float v = h < 2 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
}

float PerlinNoise::grad(int hash, float x, float y, float z) const {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = (h < 4) ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float PerlinNoise::noise(float x, float y) const {
    int X = int(floor(x)) & 255;
    int Y = int(floor(y)) & 255;

    float xf = x - floor(x);
    float yf = y - floor(y);

    float u = fade(xf);
    float v = fade(yf);

    int aa = perm[perm[X] + Y];
    int ab = perm[perm[X] + Y + 1];
    int ba = perm[perm[X + 1] + Y];
    int bb = perm[perm[X + 1] + Y + 1];

    float x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf), u);
    float x2 = lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u);

    return lerp(x1, x2, v);
}

float PerlinNoise::noise(float x, float y, float z) const {
    int X = int(floor(x)) & 255;
    int Y = int(floor(y)) & 255;
    int Z = int(floor(z)) & 255;

    float xf = x - floor(x);
    float yf = y - floor(y);
    float zf = z - floor(z);

    float u = fade(xf);
    float v = fade(yf);
    float w = fade(zf);

    int aaa = perm[perm[perm[X] + Y] + Z];
    int aba = perm[perm[perm[X] + Y + 1] + Z];
    int aab = perm[perm[perm[X] + Y] + Z + 1];
    int abb = perm[perm[perm[X] + Y + 1] + Z + 1];

    int baa = perm[perm[perm[X + 1] + Y] + Z];
    int bba = perm[perm[perm[X + 1] + Y + 1] + Z];
    int bab = perm[perm[perm[X + 1] + Y] + Z + 1];
    int bbb = perm[perm[perm[X + 1] + Y + 1] + Z + 1];

    float x1 = lerp(
        grad(aaa, xf, yf, zf), grad(baa, xf - 1, yf, zf), u
    );
    float x2 = lerp(
        grad(aba, xf, yf - 1, zf), grad(bba, xf - 1, yf - 1, zf), u
    );
    float y1 = lerp(x1, x2, v);

    float x3 = lerp(
        grad(aab, xf, yf, zf - 1), grad(bab, xf - 1, yf, zf - 1), u
    );
    float x4 = lerp(
        grad(abb, xf, yf - 1, zf - 1), grad(bbb, xf - 1, yf - 1, zf - 1), u
    );
    float y2 = lerp(x3, x4, v);

    return lerp(y1, y2, w);
}

float PerlinNoise::fbm(const glm::vec2& p, int octaves, float lac, float gain) const {
    float sum = 0;
    float amp = 1.0f;
    glm::vec2 pos = p;

    for (int i = 0; i < octaves; i++) {
        sum += amp * noise(pos);
        pos *= lac;
        amp *= gain;
    }
    return sum;
}

float PerlinNoise::fbm(const glm::vec3& p, int octaves, float lac, float gain) const {
    float sum = 0;
    float amp = 1.0f;
    glm::vec3 pos = p;

    for (int i = 0; i < octaves; i++) {
        sum += amp * noise(pos);
        pos *= lac;
        amp *= gain;
    }
    return sum;
}

float PerlinNoise::ridged(const glm::vec2& p, int octaves, float lac, float gain) const {
    float sum = 0;
    float amp = 1.0f;
    glm::vec2 pos = p;

    for (int i = 0; i < octaves; i++) {
        float n = noise(pos);
        n = 1.0f - fabs(n);   // transforma vales em picos
        sum += n * amp;

        pos *= lac;
        amp *= gain;
    }
    return sum;
}

float PerlinNoise::ridged(const glm::vec3& p, int octaves, float lac, float gain) const {
    float sum = 0;
    float amp = 1.0f;
    glm::vec3 pos = p;

    for (int i = 0; i < octaves; i++) {
        float n = noise(pos);
        n = 1.0f - fabs(n);
        sum += n * amp;

        pos *= lac;
        amp *= gain;
    }
    return sum;
}
