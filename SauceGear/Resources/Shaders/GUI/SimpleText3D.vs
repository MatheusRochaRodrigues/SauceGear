#version 330 core

layout (location = 0) in vec4 vertex;
// vertex.xy = posição local da letra
// vertex.zw = UV

out vec2 TexCoords;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
