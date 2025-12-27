#pragma once
#include <glm/glm.hpp>

// Componente ECS para marcar entidades que podem receber outline
struct OutlineComponent {
    bool enabled = true;   // habilita ou desabilita stencil/outline
    glm::vec3 outlineColor = glm::vec3(1, 1, 0); // cor da borda
    float outlineWidth = 0.05f; // espessura em escala local
};
