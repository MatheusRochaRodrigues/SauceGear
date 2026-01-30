#version 440 core
out vec4 FragColor;
in vec3 WorldPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;

    //Sanitize
    color = max(color, vec3(0.0));
    color = min(color, vec3(65504.0)); 
    if (any(isnan(color)) || any(isinf(color))) color = vec3(0.0);
       

    
    FragColor = vec4(color, 1.0);
}