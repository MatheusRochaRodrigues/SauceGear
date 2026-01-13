
#define MAX_CASCADES 16

// Cascaded shadows
layout(std140, binding = 2) uniform LightSpaceMatrices {
    mat4 lightSpaceMatrices[MAX_CASCADES];
};

uniform sampler2DArray cascadeShadowMap;
uniform float cascadePlaneDistances[MAX_CASCADES]; 
uniform int cascadeCount;   // number of frusta - 1

uniform mat4 view; 
uniform float farPlane;

// ==== Seleção do cascade pelo depth em view space ====
int GetCascadeIndex(vec3 worldPos) {
    vec4 fragPosViewSpace = view * vec4(worldPos, 1.0);
    float depth = abs(fragPosViewSpace.z);
    
    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i) {
        if (depth < cascadePlaneDistances[i]) {
            layer = i;
            break;
        }
    }
    if (layer == -1) layer = cascadeCount;  
    return layer;
}

// ==== Shadow calculation com cascades ====
float ShadowCalculationCascade(vec3 WorldPos, vec3 Normal, vec3 lightDir) {
    int layer = GetCascadeIndex(WorldPos);

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(WorldPos, 1.0); 
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;    

    if (currentDepth > 1.0) return 0.0;

    //float bias = max(0.0015 * (1.0 - dot(normalize(Normal), normalize(-lightDir))), 0.0005);
    
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == cascadeCount) bias *= 1 / (farPlane * biasModifier); 
    else bias *= 1 / (cascadePlaneDistances[layer] * biasModifier);  

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(cascadeShadowMap, 0).xy;
    for(int x=-1; x<=1; ++x) {
        for(int y=-1; y<=1; ++y) {
            float pcfDepth = texture(cascadeShadowMap, vec3(projCoords.xy + vec2(x,y) * texelSize, layer)).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}














    //2 form
  //  vec3 WorldPos = texture(gPosition, TexCoords).rgb;   

  //  int cascadeIndex = GetCascadeIndex(WorldPos); // sua função
  //  float depth = texture(cascadeShadowMap, vec3(TexCoords, cascadeIndex)).r;
  //  vec3 color2;
  //  if (cascadeIndex == 0){ 
  //      color2 = vec3(1,0,0); 
  //  }   // vermelho
  //  else if (cascadeIndex == 1){ 
  //      color2 = vec3(0,1,0);
  //  } // verde
  //  else if (cascadeIndex == 2){ 
   //     color2 = vec3(0,0,1); 
  //  }// azul
  //  else{ 
   //     color2 = vec3(1,1,0);
 //   } // amarelo

 //   FragColor = vec4(depth * color2, 1.0); // escala em tons de cinza

  //  return;

    //1 form
  //  float depth = texture(cascadeShadowMap, vec3(TexCoords, 1)).r;
   // FragColor = vec4(vec3(depth), 1.0); // escala em tons de cinza
   // return;