#version 450 core

layout(location = 0) in vec3 aPos;     // posição world-space
layout(location = 1) in vec3 aColor;   // cor da linha

uniform mat4 uVP;

out vec3 vColor;

void main()
{
    gl_Position = uVP * vec4(aPos, 1.0);
    vColor = aColor;
}
