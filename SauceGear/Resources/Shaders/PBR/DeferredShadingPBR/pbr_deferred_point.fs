//Nota: este shader assume que o passo IBL já fez o “ambient”. Aqui só somamos luz direta pontual (evita double-count).
#version 440 core
#define MAX_LIGHTS 16 

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

layout(std140, binding = 1) uniform LightData {
    Light lights[MAX_LIGHTS];
};

uniform samplerCube pointShadows[MAX_LIGHTS];  // Mapas de sombra ponto

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMRA;

uniform vec2 screenSize;
uniform vec3 viewPos;
uniform float far_plane;

flat in int instanceID;
out vec4 FragColor;
 
const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap(vec2 TexCoords)
{ 
    vec3 Normal = texture(gNormal,   TexCoords).rgb; 
    vec3 WorldPos    = texture(gPosition, TexCoords).rgb;
    vec3 tangentNormal = texture(gNormal, TexCoords).xyz * 2.0 - 1.0;
    //vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;


    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
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

  
// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculationPoint(int l, int indexMap, vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lights[l].position;

    // use the fragment to light vector to sample from the depth map    
    // float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    // closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position

    float currentDepth = length(fragToLight);
      
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;

    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;

    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(pointShadows[indexMap], fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    // display closestDepth as debug (to visualize depth cubemap)
    //FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    return shadow;
} 



void main()
{		
    vec2 uv = gl_FragCoord.xy / screenSize; 
    vec3 WorldPos    = texture(gPosition, uv).rgb;
    
    //vec3 N = getNormalFromMap(uv);                  
    vec3 N = texture(gNormal,   uv).rgb; 

    //discard fragment
    if(all(lessThan(abs(N), vec3(1e-6)))) discard;
    vec3 albedo = pow(texture(gAlbedo, uv).rgb, vec3(2.2));
    vec3 mra    = texture(gMRA, uv).rgb;
    float metallic  = mra.r;
    float roughness = clamp(mra.g, 0.04, 1.0);  //Isso garante que o roughness nunca seja menor que 0.04, evitando os fireflies que falamos
    float ao        = mra.b; 

     
    vec3 V = normalize(viewPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    Light light = lights[instanceID];
    if (light.type != 1) discard; // only point lights are allowed
    
    float shadow = 0.0;
    if (light.castS != 0 && light.indexMap >= 0)
        shadow = ShadowCalculationPoint(instanceID, light.indexMap, WorldPos); 

    //// ===================== reflectance equation
    vec3 Lo = vec3(0.0);
     
    // calculate per-light radiance
    vec3 L = normalize(light.position - WorldPos);
    vec3 H = normalize(V + L);
    float distance = length(light.position - WorldPos);
    float attenuation = 1.0 / (distance * distance);
    //vec3 radiance = light.position * attenuation;
    vec3 radiance = light.color * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
            
    vec3 numerator    = NDF * G * F;  
    float NdotL = max(dot(N,L), 0.0);
    float NdotV = max(dot(N,V), 0.0); 
    float denominator = 4.0 * NdotV * NdotL + 0.0001; // + 0.0001 to prevent divide by zero or use + 1e-5
    vec3 specular = numerator / denominator;
        
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	  

    // scale light by NdotL                 float NdotL = max(dot(N, L), 0.0);        

    // add to outgoing radiance Lo
    vec3 diffuse = kD * albedo / PI;
    Lo = (diffuse + specular) * radiance * NdotL  * (1.0 - shadow);;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    //Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    
    ////=========================Final    

    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    //vec3 ambient = vec3(0.03) * albedo * ao;
    
    //vec3 color = ambient + Lo;

    vec3 color = Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}
