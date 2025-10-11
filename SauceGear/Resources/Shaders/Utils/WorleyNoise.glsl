// Função de Worley Noise 3D
float worleyNoise(vec3 p) {
    vec3 ip = floor(p);
    vec3 fp = p - ip;
    vec3 u = fp - floor(fp);
    vec3 d = abs(u - vec3(0.5));
    return min(min(d.x, d.y), d.z);
}
