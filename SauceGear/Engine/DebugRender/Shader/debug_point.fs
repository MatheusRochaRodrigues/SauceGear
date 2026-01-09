#version 450 core

in vec2 vUV;
in vec3 vColor;
flat in float vType;

out vec4 FragColor;

void main() {
    if (vType > 0.5) {
        // circulo
        if (length(vUV) > 1.0)
            discard;
    }

    FragColor = vec4(vColor, 1.0);
}