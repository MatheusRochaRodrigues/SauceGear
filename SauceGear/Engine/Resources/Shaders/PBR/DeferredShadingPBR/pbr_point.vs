#version 440 core
layout (location=0) in vec3 aPos;

// instancing (match com SetInstanceData)
layout (location=10) in vec3 iCenter;
layout (location=11) in float iRadius;
layout (location=12) in float iIndex; // index da luz e do shadow cubemap

flat out int instanceID;
void main(){
    instanceID = int(iIndex);
    gl_Position = vec4(aPos, 1.0); // desenhamos só para emitir um fragment em toda a tela? NÃO.
// Para light volume: proj/view * (iCenter + aPos*iRadius) — porém você já está em “deferred sphere volume”.
// Se seu VS usual já faz isso, use o dele. Este é apenas “placeholder” se você reaproveita o seu VS existente.
}
