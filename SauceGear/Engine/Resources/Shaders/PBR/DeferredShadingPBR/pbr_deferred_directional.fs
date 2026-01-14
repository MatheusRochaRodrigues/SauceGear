#version 440 core

#include <PBR.glsl>
#include <Shadows.glsl>

#define MAX_LIGHTS 16

in vec2 TexCoords;

struct LightSun { 
    vec3  direction;
    vec3  color;
    float intensity;  
}; 
uniform LightSun light;   

// G-Buffer
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMRA;

uniform vec3 viewPos;

out vec4 FragColor; 




// ==== Main PBR lighting pass ====
void main()
{
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;               
    vec3 N = texture(gNormal, TexCoords).rgb;  
    if(all(lessThan(abs(N), vec3(1e-6)))) discard;

    vec3 albedo = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2));
    vec3 mra    = texture(gMRA, TexCoords).rgb;
    float metallic  = mra.r;
    float roughness = clamp(mra.g, 0.04, 1.0);
    float ao        = mra.b; 
     
    vec3 V = normalize(viewPos - WorldPos); 
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 L = normalize(light.direction);    // direcional         //-light.direction
    vec3 H = normalize(V + L);   

    float NdotL = max(dot(N,L),0.0);
    float NDF = DistributionGGX(N,H,roughness);
    float G   = GeometrySmith(N,V,L,roughness);
    vec3  F   = fresnelSchlick(max(dot(H,V),0.0), F0);

    vec3 numerator = NDF*G*F;
    float denominator = 4.0 * max(dot(N,V),0.0) * NdotL + 1e-5;
    vec3 specular = numerator / denominator;
        
    vec3 kS = F; 
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

    //float shadow = ShadowCalculationCascade(WorldPos, N, L);    
    float shadow = 0;    
    
    vec3 radiance = light.color;
    vec3 Lo = (kD*albedo/PI + specular) * radiance * NdotL * (1.0 - shadow);

    vec3 color = Lo; 
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}




