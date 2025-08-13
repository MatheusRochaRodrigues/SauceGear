#version 440 core
out vec4 FragColor;
  
uniform sampler2D albedoMap; 

uniform vec3 viewPos;

uniform float far_plane; 


#define MAX_LIGHTS 16  
 
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace; 

//in vec4 FragPosLightSpace[MAX_LIGHTS]; // opcional: se você quiser shadow mapping para todas

//Light and Shadows
struct Light {
    vec3 position;
    //vec3 direction;
    vec3 color;
    float range;
    int type; // 0 = directional, 1 = point
    mat4 lightMatrix; // usado para directional
    int castS; // se tem sombras
    int indexMap; // usado se type == 1
};

uniform Light lights[MAX_LIGHTS];
uniform int numLights;

//uniform sampler2D   shadowMaps  [MAX_LIGHTS];     // para luzes direcionais
//uniform samplerCube MapShadowPoint[MAX_LIGHTS];     // para luzes pontuais 
//uniform sampler2D   MapShadowDirectional[MAX_LIGHTS];     // para luzes direcionais
  
uniform samplerCube pointShadows[MAX_LIGHTS];     // para luzes pontuais 
uniform sampler2D   shadowMaps  [MAX_LIGHTS];     // para luzes direcionais


// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);
 

float ShadowCalculationDirectional(int i, vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMaps[i], projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lights[i].position - FragPos);                //lights[i].direction
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMaps[i], 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMaps[i], projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

float ShadowCalculationPoint(int i, vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lights[i].position;

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
        float closestDepth = texture(pointShadows[i], fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    // display closestDepth as debug (to visualize depth cubemap)
    //FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    return shadow;
}

float ShadowCalculationPointDebug(int i, vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lights[i].position;

    // use the fragment to light vector to sample from the depth map    
    float closestDepth = texture(pointShadows[i], fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
     
    // Retorna o depth normalizado só pra debug
    return closestDepth / far_plane;

    // display closestDepth as debug (to visualize depth cubemap)
    //FragColor = vec4(vec3(closestDepth / far_plane), 1.0);     
}



void main()
{           
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 baseColor = texture(albedoMap, TexCoords).rgb;
    
    vec3 result = vec3(0.0);
     
    for (int i = 0; i < numLights; ++i) {
        Light light = lights[i];
        vec3 lightDir = vec3(0.0);
        float shadow = 0.0;
        float attenuation = 1.0;

        if (light.type == 0) {
            lightDir = normalize(-light.direction);
            shadow = ShadowCalculationDirectional(i, FragPosLightSpace);


            //shadow = ShadowCalculationDirectional(i, FragPosLightSpace[i]);

        } else if (light.type == 1) {
            lightDir = normalize(light.position - FragPos);
            float dist = length(light.position - FragPos);
            attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * (dist * dist));

            if(light.castS == 1){
                result = vec3(1);
                shadow = ShadowCalculationPoint(light.indexMap, FragPos);  
            }
            //float debugValue = ShadowCalculationPointDebug(i, FragPos);
            //result = vec3(shadow);
        }

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

        vec3 ambient  = 0.2  * light.color * baseColor;
        vec3 diffuse  = diff * light.color * baseColor;
        vec3 specular = spec * light.color;

        //result += (ambient + (1.0 - shadow) * (diffuse + specular)) * attenuation;
    }

    FragColor = vec4(baseColor, 1.0); 
}