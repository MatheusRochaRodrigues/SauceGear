#version 460 core
#define CASCADE_COUNT 5 //4  // igual ao C++
layout(triangles, invocations = CASCADE_COUNT) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140, binding = 2) uniform LightSpaceMatrices {
    mat4 lightSpaceMatrices[16];
};

in VS_OUT {
    vec4 worldPos;
} gs_in[];

void main()
{
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}
