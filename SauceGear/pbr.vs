#version 330 core

layout (location = 0) in vec3 aPos;      // posição do vértice
layout (location = 1) in vec3 aNormal;   // normal do vértice
layout (location = 2) in vec2 aTexCoords; // coordenadas UV

out vec3 WorldPos;       // posição no espaço mundo (para iluminação no fragment shader)
out vec3 Normal;        // normal no espaço mundo
out vec2 TexCoords;     // repassa UV para o fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    WorldPos = vec3(model * vec4(aPos, 1.0));      // converte para world space
    Normal = mat3(transpose(inverse(model))) * aNormal;  // transforma normal corretamente
    TexCoords = aTexCoords;                        // passa UV para frente
    gl_Position = projection * view * vec4(WorldPos, 1.0); // calcula posição final do vértice
}
