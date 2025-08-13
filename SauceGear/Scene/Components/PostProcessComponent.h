#pragma once
#include "../../Graphics/Shader.h"
#include "../../Graphics/Framebuffer.h"

struct PostProcessComponent {
    Shader* shader;           // Shader do efeito       //std::shared_ptr<Shader> shader; 
    //GLuint inputTexture;      // Textura da cena renderizada
    //static inline GLuint outputFBO = 0; // Compartilhado por todos          Framebuffer de saída (ou 0 para tela) 
    //static inline Framebuffer* outputFBO = nullptr; // Compartilhado por todos          Framebuffer de saída (ou 0 para tela) 
    Framebuffer* outputFBO = nullptr; // Compartilhado por todos          Framebuffer de saída (ou 0 para tela) 

    virtual void Apply() = 0;                           //GLuint* currentTex
    virtual ~PostProcessComponent() = default;

    inline GLuint GetOutputTexture() {
        return outputFBO->GetTexture(0);
    }
    /*static inline GLuint GetOutputTexture() {
        return outputFBO->GetTexture(0);
    }*/
};
