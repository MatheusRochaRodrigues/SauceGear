

struct PBRSurface {
    vec3  albedo;
    float metallic;
    float roughness;
    float ao;
};

struct PBRData {
    vec3 N;     // Normal
    vec3 V;     // View
    vec3 L;     // Light
    vec3 H;     // Half
};
