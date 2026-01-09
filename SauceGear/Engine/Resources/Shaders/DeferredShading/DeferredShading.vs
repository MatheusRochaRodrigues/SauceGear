#version 440 core
layout (location = 0) in vec3 aPos;

layout (location = 10) in vec3 aInstancePos;
layout (location = 11) in float aRadius;
layout (location = 12) in float iD;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

flat out int instanceID;

void main() {
    instanceID = gl_InstanceID;
    vec3 worldPos = aPos * aRadius + aInstancePos;
    gl_Position = projection * view * vec4(worldPos, 1.0);
}
