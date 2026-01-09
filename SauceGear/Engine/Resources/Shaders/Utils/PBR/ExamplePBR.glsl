#include "pbr_eval.glsl"

void main() {
    PBRSurface surf;
    surf.albedo   = gAlbedo.rgb;
    surf.metallic = gMRA.r;
    surf.roughness= gMRA.g;
    surf.ao       = gMRA.b;

    PBRLighting l;
    l.N = normalize(gNormal);
    l.V = normalize(camPos - gPosition);
    l.L = normalize(lightDir);
    l.H = normalize(l.V + l.L);

    vec3 radiance = lightColor * lightIntensity;

    vec3 color = ComputePBR(surf, l, radiance);

    FragColor = vec4(color, 1.0);
}
