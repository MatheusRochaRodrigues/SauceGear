//Nota: este shader assume que o passo IBL já fez o “ambient”. Aqui só somamos luz direta pontual (evita double-count).
#version 440 core 

#include <ShadowsPoint.glsl>
#include <PBR/pbr_eval.glsl>
 
uniform sampler2D gPosition;
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gMRA;

uniform vec2 screenSize;

flat in int lightIndex;
out vec4 FragColor; 

void main()
{		 
    Light light = lights[lightIndex];                       if (light.params.x != 1) discard; // only point lights are allowed
    
    vec3  LightPos       = light.posDir_radius.xyz;
    vec3  LightColor     = light.color_intensity.xyz; 
    float LightIntensity = light.color_intensity.w; 

    //FragColor = vec4(light.params.x,light.params.x,light.params.x, 1.0);
    //return;

    vec2 uv = gl_FragCoord.xy / screenSize; 
    vec3 WorldPos    = texture(gPosition, uv).rgb;
                 
    vec3 N = texture(gNormal, uv).rgb;    //vec3 N = normalize(texture(gNormal, uv).xyz * 2.0 - 1.0);  it's not necessary'  //convert [0,1] to [-1,1]

    //discard fragment
    if(all(lessThan(abs(N), vec3(1e-6)))) discard;
    vec3  albedo    = texture(gAlbedo, uv).rgb;  //if dont be in SRGB ->  //pow(texture(gAlbedo, uv).rgb, vec3(2.2));
    vec3  mra       = texture(gMRA, uv).rgb; 

    //// ===================== reflectance equation 
    // calculate per-light radiance
    vec3 L = normalize(LightPos - WorldPos); 
    vec3 V = normalize(viewPos - WorldPos);
    
    PBRSurface surf;
    surf.albedo   = albedo.rgb;
    surf.metallic = mra.r;
    surf.roughness= mra.g;
    surf.ao       = mra.b; 

    PBRData l;
    l.N = normalize(N);
    l.V = V;
    l.L = L;
    l.H = normalize(V + L);

    float distance = length(LightPos - WorldPos);
    float attenuation = 1.0 / (distance * distance); 
    vec3 radiance = LightColor * attenuation;               //vec3 radiance = LightPos * attenuation; 
    radiance = radiance * LightIntensity; 

    vec3 Lo = ComputePBR(surf, l, radiance); 
            
    float shadow = 0.0;
    //if (light.castS != 0 && light.indexMap >= 0) shadow = ShadowCalculationPoint(lightIndex, light.indexMap, WorldPos); 
    //Lo*= ( 1 - shadow);

    FragColor = vec4(Lo, 1.0);   
}






