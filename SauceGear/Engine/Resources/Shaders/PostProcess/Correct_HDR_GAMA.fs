#version 330 core
out vec4 FragColor; 

uniform sampler2D scene;        //screenTexture
in vec2 TexCoords;  

void main() { 
    vec3 color = texture(scene, TexCoords).rgb;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));   

    FragColor = vec4(color, 1.0);
}
