#version 440 core

#include <LightType.glsl>       //esse sempre primeiro   
#include <Shadows.glsl>         //esse dps de colocar LightType

#include <PBR/pbr_eval.glsl>

#define MAX_LIGHTS 16

in vec2 TexCoords;

// Sun
uniform LightData light;  
uniform vec3 viewPos;

// G-Buffer
uniform sampler2D gPosition;    // 0
uniform sampler2D gAlbedo;      // 1
uniform sampler2D gNormal;      // 2
uniform sampler2D gMRA;         // 3
 
uniform int shadowWay;

out vec4 FragColor;   

// ==== Main PBR lighting pass ====
void main()
{   
    //Extract G-Buffer
    vec3 N          = texture(gNormal,   TexCoords).rgb;                        // N = normalize(N * 2.0 - 1.0);

    if(all(lessThan(abs(N), vec3(1e-6)))) discard;

    vec3 WorldPos   = texture(gPosition, TexCoords).rgb;   
    vec3 albedo     = texture(gAlbedo,   TexCoords).rgb;    // is already in SRGB else -> vec3 albedo     = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2))   
    vec3 mra        = texture(gMRA,      TexCoords).rgb; 

    //=======================================================================================================
    //PBR  
    PBRSurface surf;
    surf.albedo   = albedo.rgb;
    surf.metallic = mra.r;
    surf.roughness= mra.g;
    surf.ao       = mra.b;

    vec3 L = normalize(light.direction);           //-light.direction

    PBRData l;
    l.N = normalize(N);
    l.V = normalize(viewPos - WorldPos);
    l.L = L;
    l.H = normalize(l.V + l.L);

    vec3 radiance = light.color * light.intensity; 
    vec3 Lo = ComputePBR(surf, l, radiance); 
     
    // Shadow 
    float shadow = 0;
    if(shadowWay == 1) shadow = ShadowCalculationCascade(WorldPos, N, L); 
    
    Lo *= (1.0 - shadow);  
    // Out
    FragColor = vec4(Lo, 1.0); 
      
    //DEBUG
    //FragColor = vec4(normalize(N) * 0.5 + 0.5, 1.0);
    //FragColor = vec4(Lo, 1.0); 
    //return; 
}




