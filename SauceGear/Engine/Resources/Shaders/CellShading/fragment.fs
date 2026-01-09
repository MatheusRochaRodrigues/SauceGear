uniform sampler2D albedoMap;
uniform sampler2D specularMap;
uniform float roughness;

in vec2 TexCoords;
out vec4 FragColor;

void main() {
    //vec3 albedo = texture(albedoMap, TexCoords).rgb;
    //FragColor = vec4(albedo, 1.0);

    
    vec4 albedo = mix(texture(albedoMap, TexCoords), texture(specularMap, TexCoords), 0.0);
    FragColor = albedo;
}
