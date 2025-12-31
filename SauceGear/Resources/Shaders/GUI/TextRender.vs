#version 330 core
layout (location = 0) in vec4 vertex; // xy = pos, zw = uv   // <vec2 pos, vec2 tex>

uniform mat4 MVP;

out vec2 TexCoords;

void main() {
    gl_Position = MVP * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
