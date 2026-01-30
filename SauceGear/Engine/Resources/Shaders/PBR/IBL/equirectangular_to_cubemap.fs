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
    // Mata NaN/Inf primeiro
    if (any(isnan(color)) || any(isinf(color))) color = vec3(0.0); 

    // Clamp físico (não matemático)
    color = clamp(color, 0.0, 2000.0);          //outra alternativa -> 1000.0 (para IBL isso já é muito alto)
    
    // Clamp usando o max float para rgb16
    //color = min(color, vec3(65504.0)); 

    // compressão suave (REMOVE banding)
    // color = color / (color + vec3(1.0)); 

    
    FragColor = vec4(color, 1.0);
}



/* 
    //Sanitize
    color = max(color, vec3(0.0));
    color = min(color, vec3(65504.0)); 
    if (any(isnan(color)) || any(isinf(color))) color = vec3(0.0);
       
*/