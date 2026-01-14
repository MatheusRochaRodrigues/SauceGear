#include "pbr_types.glsl"
#include "brdf_ggx.glsl"

// Avaliação PBR para UMA luz
vec3 ComputePBR(
    PBRSurface surface,
    PBRLighting light,
    vec3 radiance
) {
    float NdotL = saturate(dot(light.N, light.L));
    float NdotV = saturate(dot(light.N, light.V));

    if (NdotL <= 0.0 || NdotV <= 0.0) return vec3(0.0);

    // Base reflectance
    vec3 F0 = mix(vec3(0.04), surface.albedo, surface.metallic);

    // Cook-Torrance
    float D = DistributionGGX(light.N, light.H, surface.roughness);
    float G = GeometrySmith(light.N, light.V, light.L, surface.roughness);
    vec3  F = FresnelSchlick(saturate(dot(light.H, light.V)), F0);

    vec3 numerator = D * G * F;
    float denom = 4.0 * NdotV * NdotL + 0.001;
    vec3 specular = numerator / denom;

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - surface.metallic);

    return (kD * surface.albedo / PI + specular) * radiance * NdotL;
} 