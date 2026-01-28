//pbr_ibl.fs — rápido e estável (usa irradiance + prefilter + BRDF LUT).
#version 440 core
out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gMRA;

uniform bool existSSAO;
uniform sampler2D SSAO;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D   brdfLUT;

uniform vec3 viewPos;
//uniform vec2 screenSize;

in vec2 TexCoords; // se seu fullscreen.vs não passar, gere por gl_FragCoord
 
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0-roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main(){ 
    //vec2 uv = gl_FragCoord.xy / screenSize;
    vec2 uv = TexCoords;

    vec3  pos = texture(gPosition, uv).rgb;
    vec3  N   = texture(gNormal,   uv).rgb;
    if(all(lessThan(abs(N), vec3(1e-6)))) discard; // pixel vazio do GBuffer
    N = normalize(N);
    vec3  albedo = pow(texture(gAlbedo, uv).rgb, vec3(2.2)); // linear
    vec3  mra = texture(gMRA, uv).rgb;

    float metallic  = mra.r;
    float roughness = clamp(mra.g, 0.04, 1.0);
    float ao        = mra.b;

    vec3 V = normalize(viewPos - pos);
    vec3 R = reflect(-V, N);

    // F0
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // Diffuse irradiance * Lambert
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 F = fresnelSchlickRoughness(max(dot(N,V),0.0), F0, roughness);
    vec3 kd = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = irradiance * albedo / 3.14159;

    // Specular IBL (prefilter + LUT)
    const float MAX_REFLECTION_LOD = 4.0; // deve casar com mips do prefilter
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N,V),0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 color = (kd * diffuse + specular) * ao;    //Ambient

    //vec3 color = kd * diffuse * ao + specular;
    //specular *= mix(1.0, ao, roughness);

    //SSAO
    if(existSSAO){
        color *= texture(SSAO, uv).r; // here we add occlusion factor              vec3 ambient    ==    AmbientOcclusion
    }

    FragColor = vec4(color, 1.0);                   //Ambient result
}
