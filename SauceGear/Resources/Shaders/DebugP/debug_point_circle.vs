#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uVP;
uniform vec3 uColor;
uniform float uPointSize;
out vec3 vColor;
void main() {
    gl_Position = uVP * vec4(aPos, 1.0);
    gl_PointSize = uPointSize;
    vColor = uColor;
}
