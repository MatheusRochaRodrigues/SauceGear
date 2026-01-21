#pragma once  
#include <algorithm>
#include <glm/glm.hpp>
 
enum OperationSDF
{
    SDF_OPERATION_UNION,
    SDF_OPERATION_SUBTRACTION,
    SDF_OPERATION_INTERSECTION,
    SDF_OPERATION_SMOOTH_UNION,
    SDF_OPERATION_SMOOTH_SUBTRACTION,
    SDF_OPERATION_SMOOTH_INTERSECTION,
};

static inline float apply_operation(OperationSDF op, float a, float b, float k = 1.0f)
{
    switch (op)
    {
    case SDF_OPERATION_UNION:
        return std::min(a, b);
    case SDF_OPERATION_SUBTRACTION:
        return std::max(a, -b);
    case SDF_OPERATION_INTERSECTION:
        return std::max(a, b);
    case SDF_OPERATION_SMOOTH_UNION:
    {
        float h = std::clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
        return glm::mix(b, a, h) - k * h * (1.0f - h);
    }
    case SDF_OPERATION_SMOOTH_SUBTRACTION:
    {
        float h = std::clamp(0.5f - 0.5f * (b + a) / k, 0.0f, 1.0f);
        return glm::mix(a, -b, h) + k * h * (1.0f - h);
    }
    case SDF_OPERATION_SMOOTH_INTERSECTION:
    {
        float h = std::clamp(0.5f - 0.5f * (b - a) / k, 0.0f, 1.0f);
        return glm::mix(b, a, h) + k * h * (1.0f - h);
    }
    default:
        return a;
    }
} 

/* 
⭐ Compor SDF → A parte poderosa! 
SDF é interessante não por cada forma individual, mas porque você pode combiná-las elegantemente.

União
return min(sdfA, sdfB);

Interseção
return max(sdfA, sdfB);

Subtração
return max(sdfA, -sdfB);



Exemplo: planeta com cavernas
float planet = length(p) - 1000.0;
float caves = noise3D(p * 0.02) - 0.25;

return max(planet, -caves); // interseção


Lógica:

planet < 0 → dentro do planeta 
caves < 0 → região onde ruído define cavernas 
max() mantém apenas onde ambos são negativos.





2️⃣ Qual é a representação matemática ou lógica da superfície?
Ex.: terreno

Superfície = y = height(x,z)

Ex.: esfera

Superfície = |p - center| = radius

Ex.: caverna

Superfície = um nível no ruído 3D.

Ex.: dungeon

Superfície = paredes de uma box.

3️⃣ A distância até essa superfície como é medida?

Terreno: vertical

Esfera: distância radial

Caverna: indiretamente pelo valor do ruído

Sala: distância até faces da caixa









⭐ Distância de uma caixa (super útil)

Uma caixa no centro:

float sdBox(vec3 p, vec3 b)
{
    vec3 d = abs(p) - b;
    return length(max(d, vec3(0))) + min(max(d.x,max(d.y,d.z)),0.0);
}


Isso gera ambientes perfeitamente sólidos.

*/