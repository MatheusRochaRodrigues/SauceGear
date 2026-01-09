#version 330 core
uniform vec3 uColor;
uniform bool uCircular; // se true, desenha círculo, se false, quadrado
in vec2 gl_PointCoord;
out vec4 FragColor;

void main() {
    if(uCircular){
        vec2 coord = gl_PointCoord - vec2(0.5);
        float r = length(coord);
        if(r > 0.5) discard;
    }
    FragColor = vec4(uColor, 1.0);
}
