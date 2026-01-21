#version 440 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec3 gNormal;
layout (location = 3) out vec4 gMRAO;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 NormalVertex;
in mat3 TBN;
 
uniform sampler2D Albedo;     // 1
uniform sampler2D Normal;     // 2 
uniform sampler2D Metallic;   // 3
uniform sampler2D Roughness;  // 4

//uniform sampler2D MRAO;     // OTIMIZADO

uniform bool op_Normal; 

uniform bool correctGama; 

void main() {    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    
    // and the diffuse per-fragment color
    gAlbedo.rgb = pow(texture(Albedo, TexCoords).rgb, vec3(2.2));   //convert SRGB to linear space
    //gAlbedo.rgb = texture(Albedo, TexCoords).rgb;   

    // normal map
    if (op_Normal) { 
        vec3 n =    texture(Normal, TexCoords).rgb;
        n =         normalize(n * 2.0 - 1.0);   // [0,1] -> [-1,1]
        gNormal =   normalize(TBN * n);         // tangent -> world-space
    }else{   
        gNormal = normalize(NormalVertex);  // also store the per-fragment normals into the gbuffer
    }
     
    // store specular intensity in gAlbedoSpec's alpha component 
    gMRAO = vec4(texture(Metallic, TexCoords).r, texture(Roughness, TexCoords).r, 1.0f, 1.0f);  
}