#version 450 core 

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;

// sliders
uniform float uExposure;    // 0.0 – 5.0
uniform float uSaturation;  // 0.0 – 2.0   (1.0 = neutro)
uniform float uContrast;    // 0.5 – 2.0   (1.0 = neutro)

// ACES
vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    x = max(vec3(0.0), x);
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

/*vec3 PreserveWarmColors(vec3 color) {
    float warmth = smoothstep(0.2, 1.0, color.r);
    color.r += warmth * 0.05;
    return color;
}*/


void main()
{
    // 1 HDR linear
    vec3 color = texture(scene, TexCoords).rgb;

    // 2 Exposure
    color *= uExposure;

    // 3 Saturacao (antes do ACES)
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luma), color, uSaturation);

    // 4 Contraste perceptual (pivot em luma nao 0.5!)
    color = (color - luma) * uContrast + luma;

    // 5 Tonemapping
    color = ACESFilm(color);
    //color = PreserveWarmColors(color);
    color = clamp(color, 0.0, 1.0);

    // 6 Gamma
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
