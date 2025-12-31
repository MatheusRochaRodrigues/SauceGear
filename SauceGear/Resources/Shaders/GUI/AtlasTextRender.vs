#version 330 core
layout (location=0) in vec4 quad;
layout (location=1) in vec4 instPosSize;
layout (location=2) in vec4 instUV;
layout (location=3) in vec4 instColor;

out vec2 vUV;
out vec4 vColor;

uniform mat4 MVP;

void main(){
    vec2 pos = quad.xy * instPosSize.zw + instPosSize.xy;
    vUV = mix(instUV.xy, instUV.zw, quad.zw);
    vColor = instColor;
    gl_Position = MVP * vec4(pos,0,1);
}
