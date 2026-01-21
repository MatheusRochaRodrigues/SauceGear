 
#define MAX_LIGHTS 16 
//---------------------------------------- POINT
struct Light {
    vec4 posDir_radius;   // xyz = position/direction, w = radius
    vec4 color_intensity; // rgb = color, a = intensity
    vec4 params;          // x=type, y=castShadow, z=angle, w=indexMap
    mat4 lightMatrix;     // opcional
};
//16 + 16 + 16 + 64 = 112 bytes

layout(std430, binding = 1) buffer LightData {
    Light lights[];
};
 

// Shadows
uniform samplerCube pointShadows[MAX_LIGHTS];  // Mapas de sombra ponto

uniform float far_plane;
uniform vec3 viewPos;

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
    vec3 position = lights[l].posDir_radius.xyz;
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - position;

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















//----------------------------------------------------------- LEGACY ------------------------------
 
 
struct LightExPlication {
    vec4  posDir_radius;            //posDir Vec3, radius float                         16                          //0
    vec4  color_intensity;          //color vec3, intensity float                       16                          //16  
    vec4  type_cast_angle_indexMp;  //type int, cast int, angle float, indexMp int      16                          //32
    mat4  lightMatrix;              //unused here                                       16 x 4 coluns =  64         //48
                                    //                                                  total = 112                 //64
                                                                                                                    //80
                                                                                                                    //96
                                                                                                                    //112

    //struct correct align base (The base alignment is the largest base alignment of any of its members. 
    //The offset of each member is a multiple of its alignment.)                        total(correct align base struct) = 112 size

    //mat4 = vec4 * 4 = 16 * 4
    //                                                                                  total = 112 == 7 * sizeof(vec4)
};
 

//layout(std140, binding = 1) uniform LightData {
//    Light lights[MAX_LIGHTS];
//};





struct LightLegacy {
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