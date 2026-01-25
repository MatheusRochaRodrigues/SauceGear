#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 LocalPos;        // posição real (raio)
out vec3 LocalDir;        // direção normalizada
out vec3 WorldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    LocalPos = aPos;               // posição real
    LocalDir = normalize(aPos);    // direção para ruído / plasma

    vec4 world = model * vec4(aPos, 1.0); 
    gl_Position = projection * view * world;
}



/*
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
*/