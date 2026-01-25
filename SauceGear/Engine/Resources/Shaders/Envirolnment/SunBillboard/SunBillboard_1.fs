#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec3 color;
uniform float time;

// função de ruído simples
float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1,311.7)))*43758.5453); }
// 1
float noise(vec2 p) {
    float n = sin(p.x)*sin(p.y);
    return fract(n * 43758.5453);
}

/*  2
float noise(vec2 p) {
    vec2 i = floor(p); vec2 f = fract(p);
    float a = hash(i); float b = hash(i+vec2(1.0,0.0));
    float c = hash(i+vec2(0.0,1.0)); float d = hash(i+vec2(1.0,1.0));
    vec2 u = f*f*(3.0-2.0*f);
    return mix(a,b,u.x) + (c-a)*u.y*(1.0-u.x) + (d-b)*u.x*u.y;
}
*/

void main()
{
    vec2 center = TexCoords - 0.5;
    float r = length(center) * 2.0; // 0 no centro, 1 na borda
    if (r > 1.0) discard;

    float radial = length(center);

    //WARP
    vec2 warp = center * 4.0;
    warp += vec2(
        sin(warp.y * 3.0 + time),
        cos(warp.x * 3.0 + time)
    ) * 0.12;

    float n1 = noise(warp * 8.0 + time);
    float n2 = noise(warp * 16.0 - time * 0.7);

    float turbulence = mix(n1, n2, 0.5);

    float core = exp(-radial * 4.0);
    float glow = pow(1.0 - radial, 2.8);

    float pulse = 0.9 + 0.1 * sin(time * 2.0);

    vec3 hot = mix(color * 1.4, vec3(1.0, 0.9, 0.6), core);
    vec3 finalColor = hot * (core * 2.2 + glow) * turbulence * pulse;

    float alpha = (core + glow) * 0.85;

    FragColor = vec4(finalColor, alpha);

}
