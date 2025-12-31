#version 330 core

layout (location = 0) in vec4 quad; // xy = quad pos, zw = uv

layout (location = 1) in vec4 iPosSize;
layout (location = 2) in vec4 iUV;
layout (location = 3) in vec4 iColor;
layout (location = 4) in vec4 iOutline;
layout (location = 5) in vec4 iShadowA;
layout (location = 6) in vec2 iShadowB;

uniform mat4 MVP;

out VS_OUT {
    vec2 uv;
    vec4 color;
    float outline;
    vec4 outlineColor;
    vec2 shadowOffset;
    vec4 shadowColor;
} vs;

void main() {
    vec2 pos  = iPosSize.xy + quad.xy * iPosSize.zw;
    gl_Position = MVP * vec4(pos, 0.0, 1.0);

    vs.uv = mix(iUV.xy, iUV.zw, quad.zw);

    vs.color = iColor;

    vs.outline = iOutline.x;
    vs.outlineColor = vec4(iOutline.yzw, 1.0);

    vs.shadowOffset = iShadowA.xy;
    vs.shadowColor = vec4(iShadowA.zw, iShadowB);
}
