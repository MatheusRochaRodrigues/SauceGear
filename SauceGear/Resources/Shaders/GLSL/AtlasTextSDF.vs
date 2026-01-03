#version 330 core

layout (location = 0) in vec4 quad;
layout (location = 1) in vec4 iPosSize;
layout (location = 2) in vec4 iUV;
layout (location = 3) in vec4 iColor;
layout (location = 4) in vec4 iOutline;
layout (location = 5) in vec4 iShadowA;
layout (location = 6) in vec2 iShadowB;

uniform mat4 M_VP; 
uniform vec3 uCamRight;
uniform vec3 uCamUp;
uniform int  uMode;   // 0 = 2D, 1 = 3D billboard

out VS_OUT {
    vec2 uv;
    vec4 color;
    float outline;
    vec4 outlineColor;
    vec2 shadowOffset;
    vec4 shadowColor;
} vs;

void main() {
    vec2 quadPos = vec2(quad.x, 1.0f - quad.y);  


    vec2 local = quadPos * iPosSize.zw;  //vec2 local = quad.xy * iPosSize.zw;
     
    if (uMode == 0) {
        // -------- 2D --------
        vec2 pos = iPosSize.xy + local;
        gl_Position = M_VP * vec4(pos, 0.0, 1.0);
    } else {
        // -------- 3D BILLBOARD --------
        vec3 world =
            vec3(iPosSize.xy, 0.0) +
            uCamRight * local.x +
            uCamUp    * local.y;

        gl_Position = M_VP * vec4(world, 1.0);
    }


    vs.uv = mix(iUV.xy, iUV.zw, quad.zw);
    vs.color = iColor;
    vs.outline = iOutline.x;
    vs.outlineColor = vec4(iOutline.yzw, 1.0);
    vs.shadowOffset = iShadowA.xy;
    vs.shadowColor = vec4(iShadowA.zw, iShadowB);
}
