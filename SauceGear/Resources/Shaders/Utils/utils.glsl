// utils.glsl
float saturate(float x) {
    return clamp(x, 0.0, 1.0);
}

vec3 lerp(vec3 a, vec3 b, float t) {
    return a + t * (b - a);
}
