#version 330 core

// atributos do vértice (posição do ponto unitário)
layout(location = 0) in vec3 aPos;

// atributos da instância
layout(location = 1) in vec3 iPos;      // posição do ponto
layout(location = 2) in vec3 iColor;    // cor
layout(location = 3) in float iSize;    // tamanho
layout(location = 4) in float iType;    // 0 = Square, 1 = Circle

uniform mat4 uVP;

out vec3 vColor;
flat out float vType; // flat para não interpolar tipo

void main() {
    // aplica posição da instância
    gl_Position = uVP * vec4(aPos + iPos, 1.0);
    gl_PointSize = iSize;
    vColor = iColor;
    vType = iType;
}
