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

uniform sampler2D uMSDF;
uniform float pxRange = 4.0;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange(vec2 uv) {
    vec2 duv = fwidth(uv);
    return max(0.5 * dot(duv, vec2(pxRange)), 1.0);
}

void main() {
    vec3 msdf = texture(uMSDF, fs.uv).rgb;
    float sd = median(msdf.r, msdf.g, msdf.b);

    float w = screenPxRange(fs.uv);
    float alpha = smoothstep(0.5 - w, 0.5 + w, sd);

    // -------- OUTLINE --------
    float outlineAlpha = 0.0;
    if (fs.outline > 0.0) {
        outlineAlpha = smoothstep(
            0.5 - fs.outline - w,
            0.5 - fs.outline + w,
            sd
        );
    }

    // -------- SHADOW --------
    vec3 msdfShadow = texture(uMSDF, fs.uv - fs.shadowOffset * 0.001).rgb;

    float sdShadow = median(msdfShadow.r, msdfShadow.g, msdfShadow.b);
    float shadowAlpha = smoothstep(0.5 - w, 0.5 + w, sdShadow) * fs.shadowColor.a;

    vec4 shadow = vec4(fs.shadowColor.rgb, shadowAlpha);

    // -------- COMPOSE --------
    vec4 outline = fs.outlineColor * outlineAlpha;
    vec4 fill    = fs.color * alpha;

    FragColor = shadow;
    FragColor = mix(FragColor, outline, outline.a);
    FragColor = mix(FragColor, fill, fill.a);

    if (FragColor.a <= 0.001)
        discard;
}
