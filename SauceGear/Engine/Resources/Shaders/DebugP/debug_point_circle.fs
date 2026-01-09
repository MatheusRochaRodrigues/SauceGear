#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    // optionally round point to circle:
    vec2 coord = gl_PointCoord - vec2(0.5);
    float r = length(coord);
    if (r > 0.5) discard;
    FragColor = vec4(vColor, 1.0);
}
