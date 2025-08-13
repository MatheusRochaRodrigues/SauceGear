#version 440 core

out vec4 FragColor;

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
uniform int numLights;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
uniform sampler2D albedoMap;
uniform vec3 viewPos;
uniform float far_plane;



// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);
 

float ShadowCalculationDirectional(int l, int indexMap, vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMaps[indexMap], projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lights[l].position - FragPos);                //lights[i].direction
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMaps[indexMap], 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMaps[indexMap], projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

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

float ShadowCalculationPointDebug(int l, int indexMap, vec3 fragPos)
{
    float far_plane = lights[l].range;

    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lights[l].position;

    // use the fragment to light vector to sample from the depth map    
    float closestDepth = texture(pointShadows[indexMap], fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
     
    // Retorna o depth normalizado só pra debug
    return closestDepth / far_plane;

    // display closestDepth as debug (to visualize depth cubemap)
    //FragColor = vec4(vec3(closestDepth / far_plane), 1.0);     
}

float ShadowCalculation2(int l, int indexMap, vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lights[l].position;
    // use the fragment to light vector to sample from the depth map    
    // float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    // closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    // float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    // float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;
    // PCF
    // float shadow = 0.0;
    // float bias = 0.05; 
    // float samples = 4.0;
    // float offset = 0.1;
    // for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    // {
        // for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        // {
            // for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            // {
                // float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r; // use lightdir to lookup cubemap
                // closestDepth *= far_plane;   // Undo mapping [0;1]
                // if(currentDepth - bias > closestDepth)
                    // shadow += 1.0;
            // }
        // }
    // }
    // shadow /= (samples * samples * samples);
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
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    return shadow;
}



void mainf() {
    float shadow = ShadowCalculation(0, 0, FragPos); // Verifique se a função está definida
    //shadow = ShadowCalculationPointDebug(i, light.indexMap, FragPos); // Verifique se a função está definida
    vec3 result = vec3(shadow);  

    FragColor = vec4(result, 1.0);
}


void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 baseColor = texture(albedoMap, TexCoords).rgb;

    vec3 result = vec3(0.0);

    for (int i = 0; i < numLights; ++i) {
        Light light = lights[i];
        vec3 lightDir = vec3(0.0);
        float shadow = 0.0;
        float attenuation = 1.0;

        if (light.type == 0) {              // Tipo Direcional
            lightDir = normalize(-light.position); // Ajuste para usar position no lugar de direction
            if (light.indexMap != -1) {
                vec4 FragPosLightSpace = light.lightMatrix * vec4(FragPos, 1.0);
                shadow = ShadowCalculationDirectional(i, light.indexMap, FragPosLightSpace);
                result = vec3(shadow);     
            }
        } else if (light.type == 1) {       // Tipo Ponto
            lightDir = normalize(light.position - FragPos);
            float dist = length(light.position - FragPos);
            attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * (dist * dist));

            if (light.indexMap != -1) {
                //shadow = ShadowCalculationPoint(i, light.indexMap, FragPos); // Verifique se a função está definida
                //shadow = ShadowCalculation(i, light.indexMap, FragPos); // Verifique se a função está definida
                //shadow = ShadowCalculationPointDebug(i, light.indexMap, FragPos); // Verifique se a função está definida
                //result = vec3(shadow);     
            }
        }
         
        //if (lights[1].indexMap == 1) {
         //   result = vec3(light.color);
        //}

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

        vec3 ambient  = 0.3  * light.color * baseColor;
        vec3 diffuse  = diff * light.color * baseColor;
        vec3 specular = spec * light.color;

        //result += (ambient + (1.0 - shadow) * (diffuse + specular)) * attenuation;
        //result += (ambient + (1.0 - shadow) * (diffuse + specular)) * 1;
        //result = (light.color);
    }

    FragColor = vec4(baseColor, 1.0);
}
