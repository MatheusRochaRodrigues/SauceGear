//Nota: este shader assume que o passo IBL já fez o “ambient”. Aqui só somamos luz direta pontual (evita double-count).
#version 440 core
#define MAX_LIGHTS 16 

struct Light {
    int   type;        // 1 = point
    vec3  position;
    vec3  color;
    float intensity;
    float range;
    float angle;
    int   castS;
    mat4  lightMatrix; // unused here
    int   indexMap;    // índice do shadow cubemap
};

layout(std140, binding = 1) uniform LightData {
    Light lights[MAX_LIGHTS];
};

uniform samplerCube pointShadows[MAX_LIGHTS];  // Mapas de sombra ponto

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMRA;

uniform vec2 screenSize;
uniform vec3 viewPos;
uniform float far_plane;

flat in int instanceID;
out vec4 FragColor;
 
const float PI = 3.14159265359;

// =================== PBR helpers ===================
float DistributionGGX(vec3 N, vec3 H, float roughness){
    float a  = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N,H), 0.0);
    float NdotH2 = NdotH*NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (3.14159 * denom * denom + 1e-5);
}
float GeometrySchlickGGX(float NdotV, float roughness){
    float r = roughness + 1.0;
    float k = (r*r) / 8.0; // UE4
    return NdotV / (NdotV * (1.0 - k) + k);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness){
    float NdotV = max(dot(N,V), 0.0);
    float NdotL = max(dot(N,L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}
vec3 FresnelSchlick(float cosTheta, vec3 F0){
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// PCF 20 amostras — igual sua máscara, normalizada
const vec3 OFFS[20] = vec3[](
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowPoint(int lIdx, int cubemapIndex, vec3 fragPos, vec3 lightPos){
    vec3 toLight = fragPos - lightPos;
    float current = length(toLight);
    float bias = 0.05; // ajuste fino
    float shadow = 0.0;
    int samples = 20;

    float viewDist = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDist / far_plane)) / 25.0;

    for(int i=0;i<samples;++i){
        float closest = texture(pointShadows[cubemapIndex], toLight + OFFS[i]*diskRadius).r;
        closest *= far_plane;
        shadow += (current - bias > closest) ? 1.0 : 0.0;
    }
    return shadow / float(samples);
}

void main(){
    vec2 uv = gl_FragCoord.xy / screenSize; 
    vec3 pos    = texture(gPosition, uv).rgb;
    vec3 N      = texture(gNormal,   uv).rgb;

    //discard fragment
    if(all(lessThan(abs(N), vec3(1e-6)))) discard;

    N = normalize(N); 
    vec3 albedo = pow(texture(gAlbedo, uv).rgb, vec3(2.2));
    vec3 mra    = texture(gMRA, uv).rgb;
    float metallic  = mra.r;
    float roughness = clamp(mra.g, 0.04, 1.0);  //Isso garante que o roughness nunca seja menor que 0.04, evitando os fireflies que falamos
    float ao        = mra.b;

    Light Lgt = lights[instanceID];
    if (Lgt.type != 1) discard; // only point lights are allowed

    vec3  Lvec = Lgt.position - pos;
    float dist = length(Lvec);
    vec3  L    = Lvec / max(dist, 1e-6);

    // atenuação física simples + cutoff por range
    float att = 1.0 / (1.0 + 0.09*dist + 0.032*dist*dist);
    if (Lgt.range > 0.0) att *= clamp(1.0 - dist/Lgt.range, 0.0, 1.0);

    vec3 V = normalize(viewPos - pos);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N,L), 0.0);
    float NdotV = max(dot(N,V), 0.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float  D = DistributionGGX(N,H,roughness);
    float  G = GeometrySmith(N,V,L,roughness);
    vec3   F = FresnelSchlick(max(dot(H,V),0.0), F0);

    vec3  nominator   = D * G * F;
    float denom       = 4.0 * max(NdotL,1e-4) * max(NdotV,1e-4) + 1e-5;
    vec3  specular    = nominator / denom;

    vec3 kd = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kd * albedo / 3.14159;

    float shadow = 0.0;
    //if (Lgt.castS != 0 && Lgt.indexMap >= 0)
    //    shadow = ShadowPoint(instanceID, Lgt.indexMap, pos, Lgt.position);

    vec3 radiance = Lgt.color * Lgt.intensity * att;
    vec3 Lo = (diffuse + specular) * radiance * NdotL * (1.0 - shadow);

    // gamma
    Lo = pow(Lo, vec3(1.0/2.2));
    FragColor = vec4(Lgt.color, 1.0);
    //FragColor = vec4(Lo, 1.0);
}
