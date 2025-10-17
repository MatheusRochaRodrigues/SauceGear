#version 440 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gMRAO;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 NormalVertex;
in mat3 TBN;
 
uniform sampler2D Normal;     // 0

uniform sampler2D Albedo;     // 1
uniform sampler2D Metallic;   // 2
uniform sampler2D Roughness;  // 3

//uniform sampler2D MRAO;     // OTIMIZADO

uniform bool op_Normal; 

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    
    // normal map
    if (op_Normal) { 
        vec3 n = texture(Normal, TexCoords).rgb;
        n = normalize(n * 2.0 - 1.0);  // [0,1] -> [-1,1]
        gNormal = normalize(TBN * n);  // tangent -> world-space
    }else{   
        gNormal = normalize(NormalVertex);  // also store the per-fragment normals into the gbuffer
    }

    // and the diffuse per-fragment color
    gAlbedo.rgb = texture(Albedo, TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component 
    //gMRAO = vec4(1,  texture(Metallic, TexCoords).r, texture(Roughness, TexCoords).r, 1.0f);
    gMRAO = vec4(1,  0.0f, 0.0f, 1.0f);
    //gMRAO = vec4(1,  0.0f, 1.0f, 1.0f);
}