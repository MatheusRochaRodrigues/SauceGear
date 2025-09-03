#version 440 core

struct Light {
    int type;
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float range;
    float angle;
    int castS;
    mat4 lightMatrix; // não usado para cascades, mas mantido para compatibilidade
};

uniform Light light;  
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform sampler2DArray cascadeShadowMap;
uniform float cascadePlaneDistances[16];
uniform int cascadeCount;

layout (std140, binding = 0) uniform LightSpaceMatrices {
    mat4 lightSpaceMatrices[16];
};

uniform vec3 viewPos;
uniform float far_plane;
uniform mat4 view;

in vec2 TexCoords;
out vec4 FragColor;

// ===============================
// Função: seleciona qual cascata usar
// ===============================
int SelectCascadeLayer(vec3 fragPosWorldSpace)
{
    vec4 fragPosViewSpace = view * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    for (int i = 0; i < cascadeCount; ++i) {
        if (depthValue < cascadePlaneDistances[i]) {
            return i;
        }
    }
    return cascadeCount; // último layer
}

// ===============================
// Função de cálculo de sombra CSM
// ===============================
float ShadowCalculationCascade(vec3 fragPosWorldSpace, vec3 normal)
{
    int layer = SelectCascadeLayer(fragPosWorldSpace);

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;
    if (currentDepth > 1.0)
        return 0.0;

    float bias = max(0.005 * (1.0 - dot(normal, -light.direction)), 0.0005);
    const float biasModifier = 0.5f;
    if (layer == cascadeCount)
        bias *= 1.0 / (far_plane * biasModifier);
    else
        bias *= 1.0 / (cascadePlaneDistances[layer] * biasModifier);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(cascadeShadowMap, 0).xy);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(cascadeShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r; 
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

// ===============================
// MAIN
// ===============================
void main()
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal  = texture(gNormal, TexCoords).rgb;
    vec3 Albedo  = texture(gAlbedoSpec, TexCoords).rgb;

    if (Normal == vec3(0.0)) discard;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 baseColor = Albedo;

    vec3 result = vec3(0.0);
    vec3 lightDir = vec3(0.0);
    float shadow = 0.0;

    if (light.type == 0) { // Direcional
        lightDir = normalize(-light.direction);
        shadow = ShadowCalculationCascade(FragPos, norm);
    }

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    vec3 ambient  = 0.3 * light.color * baseColor;
    vec3 diffuse  = diff * light.color * baseColor;
    vec3 specular = spec * light.color;

    result = (ambient + (1.0 - shadow) * (diffuse + specular));

    FragColor = vec4(result, 1.0);
}
