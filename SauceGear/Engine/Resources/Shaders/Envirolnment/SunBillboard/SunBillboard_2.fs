#version 330 core
layout(location = 0) in vec3 aPos;
out vec2 TexCoords;

uniform vec3 sunPos;
uniform mat4 view;
uniform mat4 projection;
uniform float size;

void main()
{
    // pega direita e cima da câmera
    // billboard: quad no mundo, sempre virado para a câmera
    vec3 camRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 camUp    = vec3(view[0][1], view[1][1], view[2][1]);

    vec3 worldPos = sunPos + (camRight * aPos.x + camUp * aPos.y) * size;

    gl_Position = projection * view * vec4(worldPos, 1.0);
    TexCoords = aPos.xy * 0.5 + 0.5; // -1..1 -> 0..1
}
