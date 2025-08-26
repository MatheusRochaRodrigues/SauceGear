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
    mat4 lightMatrix; 
};
 
uniform Light light;  
uniform sampler2D shadowMapSun;  // Mapas de sombra direcionais 

uniform vec3 viewPos;
uniform float far_plane;
  
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
   

float ShadowCalculationDirectional(vec4 fragPosLightSpace)
{
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 FragPos = texture(gPosition, TexCoords).rgb;

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMapSun, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);                //lights[i].direction
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMapSun, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMapSun, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main() {
    // retrieve data from G-buffer 
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    //float Specular = texture(gAlbedoSpec, TexCoords).a;

    if (Normal == vec3(0.0, 0.0, 0.0)) { discard; }
      
    // Math Light above
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 baseColor = Albedo;

    vec3 result = vec3(0.0); 
    vec3 lightDir = vec3(0.0);
    float shadow = 0.0;
    float attenuation = 1.0;

    if (light.type == 0) {              // Tipo Direcional
        lightDir = normalize(-light.position); // Ajuste para usar position no lugar de direction    
        vec4 FragPosLightSpace = light.lightMatrix * vec4(FragPos, 1.0);
        shadow = ShadowCalculationDirectional(FragPosLightSpace); 
        //result = vec3(shadow);  
    } 

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    vec3 ambient  = 0.3  * light.color * baseColor;
    vec3 diffuse  = diff * light.color * baseColor;
    vec3 specular = spec * light.color;

    result = (ambient + (1.0 - shadow) * (diffuse + specular)) * attenuation;   
    //result = (ambient + (1.0 - 0) * (diffuse + specular)) * attenuation;   
    
    //result = vec3(0,1,1) * (1 - shadow);

    FragColor = vec4(result, 1.0);
    //FragColor = vec4(Normal, 1.0);

}
