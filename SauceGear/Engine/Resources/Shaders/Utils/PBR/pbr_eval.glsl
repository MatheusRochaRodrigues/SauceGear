#include "PBR/pbr_types.glsl"
#include "PBR/brdf_ggx.glsl"

// Avaliação PBR para UMA luz
vec3 ComputePBR(
    PBRSurface surface,
    PBRData d,
    vec3 radiance
) {
    //surface.roughness = clamp(surface.roughness, 0.001, 1.0);       //surface.roughness = clamp(surface.roughness, 0.04, 1.0);
    surface.roughness = clamp(surface.roughness, 0.04, 1.0);  //Isso garante que o roughness nunca seja menor que 0.04, evitando os fireflies que falamos
     

   //N = normal, H = half, L = light, V = view
    float NdotH = max(dot(d.N, d.H), 0.0f);
    float NdotV = max(dot(d.N, d.V), 0.0f);
    float VdotH = max(dot(d.V, d.H), 0.0f); 
    float NdotL = max(dot(d.N, d.L), 0.0f);
    
    if (NdotL <= 0.0 || NdotV <= 0.0) return vec3(0.0);

    // Base reflectance
    vec3 F0 = mix(vec3(0.04), surface.albedo, surface.metallic);

    // * Cook-Torrance
    //based micro facets, supomos a normal da face como uma media de muitas microfacetas cujo seus alinhamentos é definido de acordo com sua respectiva rugosidade
    float NDF = DistributionGGX (NdotH, surface.roughness);   //NDF - Normal Distribution
    //aqui queremos levar em consideração que para sueprficies mais rugosas certos raio de luz nao alcançam a normal da face ou o visualizador devido o impediemnto de outras microfacetas vizinhas que impedem que alcancem o visualizador
    float G = GeometrySmith   (NdotV, NdotL, surface.roughness);
    //aqui lidamos com o efeito de fresnel onde um objeto quanto mais perto de 90 graus mais ele tende a ser reflexivo mesmo para materiais dieletricos
    vec3  F = FresnelSchlick  (VdotH, F0);

    vec3 numerator = NDF * G * F;
    float denom = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = numerator / denom;

    vec3 kS = F;
    vec3 kD = (1.0 - kS);
    kD *= (1.0 - surface.metallic);
      
    return (kD * surface.albedo / PI + specular) * radiance * NdotL;    //Lo
    
    //Debug
    //return vec3(G);
} 







vec4 fix_GAMA_HDR (vec3 color) {   
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    return vec4(color, 1.0);
}