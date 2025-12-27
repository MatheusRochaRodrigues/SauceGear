#version 450 core

layout(location = 0) in vec2 quadPos;

layout(location = 1) in vec3 inPos;
layout(location = 2) in vec3 inColor;
layout(location = 3) in float inSize;
layout(location = 4) in float inType;

uniform mat4 uVP;
uniform vec2 uViewport;

out vec2 vUV;
out vec3 vColor;
flat out float vType;

void main() {
    vec4 clip = uVP * vec4(inPos, 1.0);

    // tamanho em pixels -> NDC
    vec2 pixelSize = (inSize / uViewport) * 2.0;
    clip.xy += quadPos * pixelSize * clip.w;

    gl_Position = clip;

    vUV = quadPos;
    vColor = inColor;
    vType = inType;
}