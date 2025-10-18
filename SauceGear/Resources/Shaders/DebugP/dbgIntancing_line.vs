#version 450 core

layout(location = 0) in vec3 linePos;   // vertices da linha unitária (0 ou 1)
layout(location = 1) in vec3 start;     // instancing
layout(location = 2) in vec3 end;       // instancing
layout(location = 3) in vec3 color;     // instancing

uniform mat4 uVP;

out vec3 fragColor;

void main() {
    // linePos.x = 0 -> start, 1 -> end
    vec3 worldPos = mix(start, end, linePos.x);
    gl_Position = uVP * vec4(worldPos, 1.0);
    fragColor = color;
}
