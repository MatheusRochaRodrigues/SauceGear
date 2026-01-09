#version 440 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
 
layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out vec3 FragPos;
out vec2 TexCoords;
out vec3 NormalVertex;
out mat3 TBN;

uniform mat4 model; 

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz; 
    TexCoords = aTexCoords;
    
    //mat3 normalMatrix = transpose(inverse(mat3(model)));
    //vec3 N = normalize(normalMatrix * aNormal);
    //vec3 T = normalize(normalMatrix * aTangent);
    //vec3 B = normalize(normalMatrix * aBitangent);
    //TBN = mat3(T, B, N);

    

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    
    NormalVertex = normalMatrix * aNormal;                    //forma tradicional sem normal map 

    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = transpose(mat3(T, B, N));  


    gl_Position = projection * view * worldPos;
}