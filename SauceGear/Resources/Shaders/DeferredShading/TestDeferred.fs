#version 440 core

out vec4 FragColor;
 

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
 
uniform vec2 screenSize;
 
void main() {
    // retrieve data from G-buffer
    vec2 TexCoords = gl_FragCoord.xy / screenSize;
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    //float Specular = texture(gAlbedoSpec, TexCoords).a;

    if (Normal == vec3(0.0, 0.0, 0.0)) { discard; }
     
     
    //FragColor = vec4(baseColor, 1.0);
    FragColor = vec4(Albedo, 1.0);
}
