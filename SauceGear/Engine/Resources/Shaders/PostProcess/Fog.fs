#version 450 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D u_SceneColor;
uniform sampler2D u_Position;

uniform vec3 u_CamPos;
uniform float u_FogDensity;
uniform float u_FogStart;
uniform float u_FogEnd;
uniform vec3 u_FogColorNear;
uniform vec3 u_FogColorFar;

void main() {
    vec3 sceneColor = texture(u_SceneColor, TexCoords).rgb;
    vec3 worldPos = texture(u_Position, TexCoords).rgb;

    float linearDepth = length(worldPos - u_CamPos);

    float fogFactor = clamp((linearDepth - u_FogStart) / (u_FogEnd - u_FogStart), 0.0, 1.0);
    vec3 fogColor = mix(u_FogColorNear, u_FogColorFar, fogFactor);

    FragColor = vec4(mix(sceneColor, fogColor, fogFactor * u_FogDensity), 1.0);
}
