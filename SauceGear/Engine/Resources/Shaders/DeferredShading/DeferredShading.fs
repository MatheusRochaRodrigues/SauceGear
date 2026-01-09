#version 440 core

#define MAX_LIGHTS 16   
// Definição da estrutura Light fora do buffer
struct Light {
    int type;
    vec3 position;
    vec3 color;
    float intensity;
    float range;
    float angle;
    int castS;
    mat4 lightMatrix;
    int indexMap;
};

layout(std140, binding = 1) uniform LightData {
    Light lights[MAX_LIGHTS];  // Buffer contendo a lista de luzes
};  

uniform sampler2D shadowMaps[MAX_LIGHTS];  // Mapas de sombra direcionais
uniform samplerCube pointShadows[MAX_LIGHTS];  // Mapas de sombra ponto

uniform vec3 viewPos;
uniform float far_plane; 

flat in int instanceID;
out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
 
uniform vec2 screenSize;

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculationPoint(int l, int indexMap, vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lights[l].position;

    // use the fragment to light vector to sample from the depth map    
    // float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    // closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position

    float currentDepth = length(fragToLight);
      
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;

    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;

    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(pointShadows[indexMap], fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    // display closestDepth as debug (to visualize depth cubemap)
    //FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    return shadow;
} 


float ShadowCalculation(int l, int indexMap, vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lights[l].position;
    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(pointShadows[indexMap], fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    //return shadow;
    
    return (closestDepth / far_plane);
}  

void main() {
    // retrieve data from G-buffer
    vec2 TexCoords = gl_FragCoord.xy / screenSize;
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    //float Specular = texture(gAlbedoSpec, TexCoords).a;

    if (Normal == vec3(0.0, 0.0, 0.0)) { discard; }
      
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 baseColor = Albedo;

    vec3 result = vec3(0.0);
     
    Light light = lights[instanceID];
    vec3 lightDir = vec3(0.0);
    float shadow = 0.0;
    float attenuation = 1.0;

    if (light.type == 0) {              // Tipo Direcional
        lightDir = normalize(-light.position); // Ajuste para usar position no lugar de direction
        if (light.indexMap != -1) {
            //vec4 FragPosLightSpace = light.lightMatrix * vec4(FragPos, 1.0);
            //shadow = ShadowCalculationDirectional(i, light.indexMap, FragPosLightSpace);
            //result = vec3(shadow);     
        }
    } else if (light.type == 1) {       // Tipo Ponto
        lightDir = normalize(light.position - FragPos);
        float dist = length(light.position - FragPos);
        attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * (dist * dist));

        if (light.indexMap != -1) {
            shadow = ShadowCalculationPoint(instanceID, light.indexMap, FragPos); // Verifique se a função está definida
            //shadow = ShadowCalculation(instanceID, light.indexMap, FragPos); // Verifique se a função está definida
            //shadow = ShadowCalculationPointDebug(instanceID, light.indexMap, FragPos); // Verifique se a função está definida
            //result = vec3(shadow);     
        }
    } 

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    vec3 ambient  = 0.3  * light.color * baseColor;
    vec3 diffuse  = diff * light.color * baseColor;
    vec3 specular = spec * light.color;

    result = (ambient + (1.0 - shadow) * (diffuse + specular)) * attenuation;   
    
    FragColor = vec4(result, 1.0);

}
