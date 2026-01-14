#pragma once
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

    if (NdotL <= 0.0 || NdotV <= 0.0)
        return vec3(0.0);

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










//to DEBUG
struct PBRResult {
    vec3 color;
    vec3 diffuse;
    vec3 specular;
    vec3 F0;
    float NdotL;
};

PBRResult ComputePBR_Debug( PBRSurface surface, PBRLighting light, vec3 radiance ) {
    PBRResult r;

    float NdotL = saturate(dot(light.N, light.L));
    float NdotV = saturate(dot(light.N, light.V));

    r.NdotL = NdotL;

    if (NdotL <= 0.0 || NdotV <= 0.0) {
        r.color = vec3(0.0);
        r.diffuse = vec3(0.0);
        r.specular = vec3(0.0);
        r.F0 = vec3(0.0);
        return r;
    }

    r.F0 = mix(vec3(0.04), surface.albedo, surface.metallic);

    float D = DistributionGGX(light.N, light.H, surface.roughness);
    float G = GeometrySmith(light.N, light.V, light.L, surface.roughness);
    vec3  F = FresnelSchlick(saturate(dot(light.H, light.V)), r.F0);

    vec3 numerator = D * G * F;
    float denom = 4.0 * NdotV * NdotL + 0.001;
    r.specular = numerator / denom;

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - surface.metallic);

    r.diffuse = kD * surface.albedo / PI;

    r.color = (r.diffuse + r.specular) * radiance * NdotL;

    return r;
}