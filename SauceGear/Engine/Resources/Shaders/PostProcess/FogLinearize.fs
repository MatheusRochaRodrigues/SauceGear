#version 450 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D u_SceneColor;
uniform sampler2D u_Depth;

uniform float u_FogDensity;
uniform float u_FogStart;
uniform float u_FogEnd;

uniform vec3 u_FogColorNear;
uniform vec3 u_FogColorFar;

uniform mat4 u_InvProjection;
uniform mat4 u_InvView;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * 0.1 * 1000.0) / (1000.0 + 0.1 - z * (1000.0 - 0.1));
}

void main() {
    vec3 sceneColor = texture(u_SceneColor, TexCoords).rgb;
    float depth = texture(u_Depth, TexCoords).r;

    float linearDepth = LinearizeDepth(depth);

    float fogFactor = clamp(
        (linearDepth - u_FogStart) / (u_FogEnd - u_FogStart),
        0.0, 1.0
    );

    vec3 fogColor = mix(u_FogColorNear, u_FogColorFar, fogFactor);

    FragColor = vec4(mix(sceneColor, fogColor, fogFactor * u_FogDensity), 1.0);
}
