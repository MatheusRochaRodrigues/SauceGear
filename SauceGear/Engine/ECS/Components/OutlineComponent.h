#pragma once
#include <glm/glm.hpp>
#include "../Reflection/Macros.h"

// Componente ECS para marcar entidades que podem receber outline
struct OutlineComponent {
    bool enabled = true;
    glm::vec3 color = { 1, 0.6f, 0 };
    float thickness = 1.03f; // escala ou largura
     
    // REFLECTION 
    REFLECT_CLASS(OutlineComponent) {
        REFLECT_HEADER("Outline");
        REFLECT_FIELD(enabled);
        REFLECT_FIELD(color);
        REFLECT_FIELD(thickness);

        REFLECT_ADD_COMPONENT();
    } 
};

