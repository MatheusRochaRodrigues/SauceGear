#pragma once

struct PBRSurface {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

struct PBRLighting {
    vec3 N;
    vec3 V;
    vec3 L;
    vec3 H;
};
