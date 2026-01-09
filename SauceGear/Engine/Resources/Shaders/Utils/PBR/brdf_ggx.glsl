#pragma once
#include "math.glsl"

// ---------------- GGX NDF ----------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;

    float NdotH  = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

// ---------------- Geometry ----------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) * 0.125; // /8

    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float ggxV = GeometrySchlickGGX(saturate(dot(N, V)), roughness);
    float ggxL = GeometrySchlickGGX(saturate(dot(N, L)), roughness);
    return ggxV * ggxL;
}

// ---------------- Fresnel ----------------
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
