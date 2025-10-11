#version 330 core

in vec3 vColor;
flat in float vType;
out vec4 FragColor;

void main() {
    if(vType > 0.5) {
        // ponto circular
        vec2 coord = gl_PointCoord - vec2(0.5);
        float r = length(coord);
        if(r > 0.5) discard;
    }
    // se Square, desenha diretamente
    FragColor = vec4(vColor, 1.0);
}
