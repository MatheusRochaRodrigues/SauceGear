#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 LocalPos;
in vec3 LocalDir;

uniform vec3 color;
uniform float time;

// -------- util --------
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
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

void main()
{
    FragColor = vec4(color, 1.0);
}
    
/*
void main()
{ 
    //r = length(LocalPos) varia de 0 (centro) até +-1, mas não exatamente 1 porque é uma esfera unitária, 
    //nao circunscrita a uma cube de 1. O vértice mais distante tem r = 0.707 (raio da esfera inscribed na cube de -1..1).
    float maxR = 0.70710678; // raio máximo da sua esfera unitária  
    float r = length(LocalPos);     // atual raio da esfera
    float rNormalized = r / maxR; // normaliza para 0..1  -> agora vai de 0 (centro) a 1 (borda)

    // borda suave
    float alpha = smoothstep(1.0, 0.6, rNormalized); 
    if (alpha <= 0.001) discard;

    // pulsação leve
    float pulse = 0.9 + 0.1 * sin(time * 2.0);

    // ruído animado (plasma)
    float n = noise(LocalDir.xy * 4.0 + time * 0.5);
    float turbulence = mix(0.85, 1.25, n);
    
    // gradiente térmico: core quente + glow externo
    float core = exp(-rNormalized * 3.5);
    float glow = pow(1.0 - rNormalized, 3.0);   // glow mais suave

    vec3 hotColor = mix(color * 1.4, vec3(1.0, 0.9, 0.6), core);
    vec3 finalColor = hotColor * (core*2.0 + glow) * pulse * turbulence;

    FragColor = vec4(finalColor, alpha * 0.85);
}  
*/
 