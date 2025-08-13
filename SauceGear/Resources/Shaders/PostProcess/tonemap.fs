#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D u_Input;

void main() {
    vec3 hdrColor = texture(u_Input, TexCoords).rgb;
    // Tonemap simples
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    FragColor = vec4(mapped, 1.0);
}
