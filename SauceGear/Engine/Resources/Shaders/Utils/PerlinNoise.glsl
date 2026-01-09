// Função de hash para o Perlin Noise
int permute(int x) {
    return mod(x * 34 + 1, 289);
}

// Função de interpolação suave
float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// Função de gradiente
float grad(int hash, vec3 p) {
    int h = hash & 15;
    float u = h < 8 ? p.x : p.y;
    float v = h < 4 ? p.y : h == 12 || h == 14 ? p.x : p.z;
    return ((h & 1 ? -1.0 : 1.0) * (u + v));
}

// Função principal de Perlin Noise 3D
float perlinNoise(vec3 p) {
    vec3 pi = floor(p);
    vec3 pf = p - pi;
    vec3 fade_pf = fade(pf);
    int i = int(pi.x) + int(pi.y) * 57 + int(pi.z) * 113;
    int aa = permute(i);
    int ab = permute(i + 1);
    int ba = permute(i + 57);
    int bb = permute(i + 57 + 1);
    return mix(mix(mix(grad(aa, pf), grad(ab, pf - vec3(1.0, 0.0, 0.0)), fade_pf.x),
                   mix(grad(ba, pf - vec3(0.0, 1.0, 0.0)), grad(bb, pf - vec3(0.0, 1.0, 0.0) - vec3(1.0, 0.0, 0.0)), fade_pf.x), fade_pf.y),
               fade_pf.z);
}
