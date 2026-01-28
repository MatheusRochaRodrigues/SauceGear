#version 330 core
in vec2 TexCoords;

out float FragColor; 

uniform sampler2D gPosition;
uniform sampler2D gNormal;

uniform sampler2D texNoise;

uniform vec3 samples[64];   //64

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
uniform int kernelSize;      //int kernelSize = 32;        //64
uniform float radius;        //float radius = 0.5;
uniform float bias;          //float bias = 0.025;         //float bias = 0.005;
uniform float power;           

// tile noise texture over screen based on screen dimensions divided by noise size
uniform vec2 noiseScale;                // = vec2(800.0/4.0, 600.0/4.0); 

uniform mat4 view;
uniform mat4 projection;

void main()
{
    // get input for SSAO algorithm 
    vec3 worldPos = texture(gPosition, TexCoords).xyz;
    vec3 worldNormal = texture(gNormal, TexCoords).xyz;
    // convert to view space
    vec3 fragPos = (view * vec4(worldPos, 1.0)).xyz;
    vec3 normal  = normalize(mat3(view) * worldNormal);


    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        vec3 sampleWorldPos = texture(gPosition, offset.xy).xyz; // get depth value of kernel sample
        float sampleDepth   = (view * vec4(sampleWorldPos, 1.0)).z; 
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    FragColor = pow(occlusion, power);

    //FragColor = occlusion;
}