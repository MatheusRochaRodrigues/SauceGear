#version 330 core

in VS_OUT {
    vec2 uv;
    vec4 color;
    float outline;
    vec4 outlineColor;
    vec2 shadowOffset;
    vec4 shadowColor;
} fs;

out vec4 FragColor;

uniform sampler2D uAtlas;

float SampleSDF(vec2 uv) {
    return texture(uAtlas, uv).r;
}

void main() {
    // -------- BASE --------
    float dist = SampleSDF(fs.uv);
    float w = fwidth(dist);
    float alpha = smoothstep(0.5 - w, 0.5 + w, dist);

    // -------- OUTLINE --------
    float outlineAlpha = 0.0;
    if (fs.outline > 0.0) {
        outlineAlpha = smoothstep(
            0.5 - fs.outline - w,
            0.5 - fs.outline + w,
            dist
        );
    }

    // -------- SHADOW --------
    vec2 shadowUV = fs.uv - fs.shadowOffset / textureSize(uAtlas, 0);
    float sdShadow = SampleSDF(shadowUV);
    float shadowAlpha =
        smoothstep(0.5 - w, 0.5 + w, sdShadow) * fs.shadowColor.a;

    vec4 shadow = vec4(fs.shadowColor.rgb, shadowAlpha);
    vec4 outline = fs.outlineColor * outlineAlpha;
    vec4 fill = fs.color * alpha;

    // -------- COMPOSE --------
    FragColor = shadow;
    FragColor = mix(FragColor, outline, outline.a);
    FragColor = mix(FragColor, fill, fill.a); 

    if (FragColor.a < 0.01) discard;
}
