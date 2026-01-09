#version 440 core

in vec3 vDir;
out vec4 FragColor;

uniform samplerCube environmentMap;

void main() {
    vec3 dir = normalize(vDir);
    vec3 col = textureLod(environmentMap, dir, 0.0).rgb; // sky nítido
    FragColor = vec4(col, 1.0);
}
