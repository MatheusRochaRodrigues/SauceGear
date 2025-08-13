#version 440 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;


layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

//layout(std140, binding = 0) uniform GlobalUniforms {
//    mat4 view;
//    mat4 projection;
//    float time;
//    vec3 cameraPosition;
//};

//uniform mat4 projection;
//uniform mat4 view;
 
uniform mat4 model;

//uniform mat4 lightSpaceMatrix;

//uniform bool reverse_normals;

//uniform mat4 lightMatrices[MAX_LIGHTS]; // para directional lights
 
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

//out vec4 FragPosLightSpace;                   //out vec4 FragPosLightSpace[MAX_LIGHTS]; 


void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
      
    Normal = transpose(inverse(mat3(model))) * aNormal;

    TexCoords = aTexCoords;

    //for (int i = 0; i < MAX_LIGHTS; ++i) FragPosLightSpace[i] = lightMatrices[i] * vec4(FragPos, 1.0);
    //FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);


    gl_Position = projection * view * model * vec4(aPos, 1.0);
}