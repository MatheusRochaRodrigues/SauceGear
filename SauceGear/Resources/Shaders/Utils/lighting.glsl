// lighting.glsl
vec3 calculateDiffuse(vec3 normal, vec3 lightDir, vec3 lightColor) {
    float diff = max(dot(normal, lightDir), 0.0);
    return diff * lightColor;
}



//$include "math"
//$include "lighting.glsl"
