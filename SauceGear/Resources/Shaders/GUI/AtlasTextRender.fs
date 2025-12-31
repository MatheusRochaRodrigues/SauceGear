#version 330 core
in vec2 vUV;
in vec4 vColor;
out vec4 Frag;

uniform sampler2D fontAtlas;

void main(){
    float d = texture(fontAtlas,vUV).r;
    float alpha = smoothstep(0.4,0.6,d);
    Frag = vec4(vColor.rgb, vColor.a*alpha);
}
