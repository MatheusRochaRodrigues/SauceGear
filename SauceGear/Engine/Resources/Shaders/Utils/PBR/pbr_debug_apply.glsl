#include "pbr_debug.glsl"

vec3 ApplyPBRDebug(
    int mode,
    PBRSurface surface,
    PBRLighting light,
    PBRResult r
) {
    switch (mode) {

    case PBR_DEBUG_ALBEDO:
        return surface.albedo;

    case PBR_DEBUG_NORMAL:
        return light.N * 0.5 + 0.5;

    case PBR_DEBUG_ROUGHNESS:
        return vec3(surface.roughness);

    case PBR_DEBUG_METALLIC:
        return vec3(surface.metallic);

    case PBR_DEBUG_AO:
        return vec3(surface.ao);

    case PBR_DEBUG_NDOTL:
        return vec3(r.NdotL);

    case PBR_DEBUG_SPECULAR:
        return r.specular;

    case PBR_DEBUG_DIFFUSE:
        return r.diffuse;

    case PBR_DEBUG_F0:
        return r.F0;

    default:
        return r.color;
    }
}


//example
/*

#include "include/pbr_eval.glsl"
#include "include/pbr_debug_apply.glsl"

uniform int uPBRDebugMode;

void main() {
    PBRSurface surf = ...;
    PBRLighting light = ...;

    vec3 radiance = lightColor * intensity;

    PBRResult res = ComputePBR(surf, light, radiance);

    vec3 finalColor = ApplyPBRDebug(
        uPBRDebugMode,
        surf,
        light,
        res
    );

    FragColor = vec4(finalColor, 1.0);
}

*/