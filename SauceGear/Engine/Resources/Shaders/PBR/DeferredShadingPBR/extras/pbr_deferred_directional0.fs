//Nota: este shader assume que o passo IBL já fez o “ambient”. Aqui só somamos luz direta pontual (evita double-count).
#version 440 core
#define MAX_LIGHTS 16 

in vec2 TexCoords;

struct Light {
    int   type;        // 1 = point
    vec3  position;
    vec3  color;
    float intensity;
    float range;
    float angle;
    int   castS;
    mat4  lightMatrix; // unused here
    int   indexMap;    // índice do shadow cubemap
};

uniform Light light;  
uniform sampler2D shadowMapSun;  // Mapas de sombra direcionais  

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMRA;
 
uniform vec3 viewPos;
uniform float far_plane;
 
out vec4 FragColor;
 
const float PI = 3.14159265359; 

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 


float ShadowCalculationDirectional(vec4 fragPosLightSpace, vec3 FragPos, vec3 Normal)
{  
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMapSun, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);                //lights[i].direction
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMapSun, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMapSun, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}
 

void main()
{		 
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;               
    vec3 N = texture(gNormal, TexCoords).rgb;  
    if(all(lessThan(abs(N), vec3(1e-6)))) discard;  //discard fragment

    vec3 albedo = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2));
    vec3 mra    = texture(gMRA, TexCoords).rgb;
    float metallic  = mra.r;
    float roughness = clamp(mra.g, 0.04, 1.0);  //Isso garante que o roughness nunca seja menor que 0.04, evitando os fireflies que falamos
    float ao        = mra.b; 
     
    vec3 V = normalize(viewPos - WorldPos); 
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);  

    //// ===================== reflectance equation   
    vec3 L = normalize(-light.position);   // Ajuste para usar position no lugar de direction 
    vec3 H = normalize(V + L);   

    // Cook-Torrance BRDF 
    float NdotL = max(dot(N,L),0.0);
    float NDF = DistributionGGX(N,H,roughness);
    float G   = GeometrySmith(N,V,L,roughness);
    vec3 F    = fresnelSchlick( max(dot(H,V),0.0), F0);

    vec3 numerator = NDF*G*F;
    float denominator = 4.0 * max(dot(N,V),0.0) * NdotL + 0.0001;    // + 0.0001 to prevent divide by zero or use + 1e-5
    vec3 specular = numerator / denominator;
        
    // kS is equal to Fresnel
    vec3 kS = F; 
    vec3 kD = vec3(1.0) - kS; 
    kD *= 1.0 - metallic;	   
    
    float shadow = 0.0;  
    if (light.castS != 0)  {          // Tipo Direcional 
        vec4 FragPosLightSpace = light.lightMatrix * vec4(WorldPos, 1.0);
        shadow = ShadowCalculationDirectional(FragPosLightSpace, WorldPos, N);
    } 
    
    //vec3 radiance = light.color * light.intensity;
    vec3 radiance = light.color ;
    vec3 Lo = (kD*albedo/PI + specular) * radiance * NdotL * (1.0 - shadow);
    ////=========================Final    

    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    //vec3 ambient = vec3(0.03) * albedo * ao; 
    //vec3 color = ambient + Lo;

    vec3 color = Lo; // já assume que IBL/ambient foi feito em outro passo
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}
