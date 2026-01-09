#pragma once
#include <glad/glad.h>

//ATENÇÃO
//nao esqueca que alterar aq deve obrigatoriamente altera no system daynight pois ele estao com tamanho e mips fixos manualmente
struct IBLSet {
    GLuint envCubemap = 0; // 512x512 RGB16F + mipmap
    GLuint irradiance = 0; // 32x32  RGB16F
    GLuint prefilter = 0; // 128x128 RGB16F + mips
    GLuint brdfLUT = 0; // 512x512 RG16F 2D
    bool   valid() const { return envCubemap && irradiance && prefilter && brdfLUT; }

    GLuint debugFace = 0;


    void destroy() {
        if (envCubemap) glDeleteTextures(1, &envCubemap);
        if (irradiance) glDeleteTextures(1, &irradiance);
        if (prefilter)  glDeleteTextures(1, &prefilter);
        if (brdfLUT)    glDeleteTextures(1, &brdfLUT);
        if (debugFace)  glDeleteTextures(1, &debugFace);

        envCubemap = irradiance = prefilter = brdfLUT = debugFace = 0;
    }


    int width = 0;       // base width do cubemap
    int mipLevels = 1;   // número de mipmaps do prefilter
};
