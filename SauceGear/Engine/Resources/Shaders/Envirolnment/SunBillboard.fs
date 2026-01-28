#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec3 color;      // cor do sol (LDR, picker)
uniform float intensity; // 50 ~ 200
uniform float time;

// =====================
// Noise
// =====================
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1,311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = (p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

float noiseOp(vec2 p) {
    vec2 warp = p * 3.0;
    warp += vec2(
        noise(warp + time),
        noise(warp + time + 3.7)
    ) * 0.15;

    return noise(warp * 6.0);
}

// =====================
// MAIN
// =====================
void main()
{
    vec2 center = TexCoords - 0.5;
    float r = length(center) * 2.0;
    if (r > 1.0) discard;

    // pulsação
    float pulse = 0.9 + 0.1 * sin(time * 2.0);

    float n = noiseOp(center);
    float turbulence = mix(0.85, 1.15, n);

    // máscaras físicas
    float core = exp(-r * 4.0);            // núcleo energético
    float glow = pow(1.0 - r, 2.2);        // decaimento

    // =====================
    // COR HDR DO SOL
    // =====================
    vec3 hdrSun = color * intensity;

    // viés quente (preserva dourado no ACES)
    hdrSun.r *= 1.05;
    hdrSun.g *= 0.98;

    // energia final
    vec3 energy =
        hdrSun *
        (core * 2.5 + glow * 1.2) *
        pulse *
        turbulence;

    // alpha APENAS para recorte suave
    float alpha = smoothstep(1.0, 0.7, r);

    FragColor = vec4(energy, alpha);
}








/*
vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec3 InverseACES(vec3 x)
{
    return x / max(1e-4, (1.0 - x));
}



    // =====================
    // PRE-EXPOSE (ANTI TONEMAP)
    // =====================

    // inverso do ACES (aproximado)
    //energy = energy / max(vec3(1e-4), (vec3(1.0) - energy));

    // inverso da gamma
    //energy = pow(energy, vec3(2.2));

*/