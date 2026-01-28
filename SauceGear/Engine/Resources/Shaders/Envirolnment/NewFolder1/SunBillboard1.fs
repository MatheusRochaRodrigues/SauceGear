#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec3 color;
uniform float time;

uniform float intensity;

// função de ruído simples
float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1,311.7)))*43758.5453); }

float noise(vec2 p) {
    vec2 i = floor(p); vec2 f = fract(p);
    float a = hash(i); float b = hash(i+vec2(1.0,0.0));
    float c = hash(i+vec2(0.0,1.0)); float d = hash(i+vec2(1.0,1.0));
    vec2 u = f*f*(3.0-2.0*f);
    return mix(a,b,u.x) + (c-a)*u.y*(1.0-u.x) + (d-b)*u.x*u.y;
}

float noiseF(vec2 p) {
    vec2 i = p; vec2 f = fract(p);
    float a = hash(i); float b = hash(i+vec2(1.0,0.0));
    float c = hash(i+vec2(0.0,1.0)); float d = hash(i+vec2(1.0,1.0));
    vec2 u = f*f*(3.0-2.0*f);
    return mix(a,b,u.x) + (c-a)*u.y*(1.0-u.x) + (d-b)*u.x*u.y;
}

float smoothNoise(vec2 p) {
    float n = sin(p.x)*sin(p.y);
    return fract(n * 43758.5453);
}


float noiseOp(vec2 center)
{ 
// 1 
    float n = smoothNoise(center * 12.0 + time);

//2
    //float n = noise(center * 4.0 + time*0.5);

/*
//3
    vec2 warp = center * 3.0;
    warp += vec2(
        noise(warp + time),
        noise(warp + 3.7 + time)
    ) * 0.15;
    
    float n = noise(warp * 6.0);
*/
    return n;
};


void main()
{
    vec2 center = TexCoords - 0.5;
    float r = length(center) * 2.0; // 0 no centro, 1 na borda
    if (r > 1.0) discard;

    // pulsação
    float pulse = 0.85 + 0.15 * sin(time * 2.0);
    
    float n = noiseOp(center);

    float turbulence = mix(0.8, 1.2, n);

    // core e glow
    float core = exp(-r*3.5);
    float glow = pow(1.0-r, 2.5);


    vec3 sunColor = color * intensity; // intensity 50 ~ 200


    vec3 finalColor = mix(sunColor * 1.4, vec3(1.0,0.9,0.6), core) * (core*2.0+glow) * pulse * turbulence;

    FragColor = vec4(finalColor, (core + glow) * 0.85);
}
