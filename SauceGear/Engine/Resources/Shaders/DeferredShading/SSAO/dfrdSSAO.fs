#version 440 core
out float FragColor;

uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform sampler2D texNoise;

uniform mat4 projection;
uniform mat4 view;

uniform vec3 samples[64];

in vec2 TexCoords;

const float radius = 0.5;
const float bias = 0.025;

vec3 ReconstructPosition(float depth, vec2 uv)
{
    float z = depth * 2.0 - 1.0;
    vec4 clip = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 viewPos = inverse(projection) * clip;
    return viewPos.xyz / viewPos.w;
}

void main()
{
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    float depth = texture(gDepth, TexCoords).r;
    vec3 fragPos = ReconstructPosition(depth, TexCoords);

    vec3 randomVec = texture(texNoise, TexCoords * 4.0).xyz;

    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;

    for (int i = 0; i < 64; ++i) {
        vec3 samplePos = fragPos + TBN * samples[i] * radius;

        vec4 offset = projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = texture(gDepth, offset.xy).r;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));

        if (sampleDepth < offset.z - bias)
            occlusion += rangeCheck;
    }

    FragColor = 1.0 - (occlusion / 64.0);
}
