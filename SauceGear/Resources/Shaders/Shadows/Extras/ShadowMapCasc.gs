#version 460 core

#define CASCADE_COUNT 4    // igual ao C++
layout(triangles, invocations = CASCADE_COUNT + 1) in;	//4 //5
layout(triangle_strip, max_vertices = 3) out;
 
layout(std140, binding = 2) uniform LightSpaceMatrices {
    mat4 lightSpaceMatrices[16];
}; 
/*
uniform mat4 lightSpaceMatrices[16];
*/

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