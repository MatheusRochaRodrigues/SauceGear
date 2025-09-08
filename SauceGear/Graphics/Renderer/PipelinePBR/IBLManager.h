#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

// Encapsula cubemaps e LUT gerados a partir de um HDR e faz cache em disco.

//ATENÇÃO
//nao esqueca que alterar aq deve obrigatoriamente altera no system daynight pois ele estao com tamanho e mips fixos manualmente
struct IBLSet {
    GLuint envCubemap   = 0; // 512x512 RGB16F + mipmap
    GLuint irradiance   = 0; // 32x32  RGB16F
    GLuint prefilter    = 0; // 128x128 RGB16F + mips
    GLuint brdfLUT      = 0; // 512x512 RG16F 2D
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

class Shader;

//IBLManager (gera / carrega / salva IBL)
class IBLManager {
public:
    // Gera ou carrega do cache.
    // cacheDir: Resources/Cache/IBL/
    // Retorna IBLSet preenchido.
    static IBLSet EnsureIBL(const std::string& hdrPath,
        const std::string& cacheDir,
        Shader& shEquirect,
        Shader& shIrradiance,
        Shader& shPrefilter,
        Shader& shBRDF,
        GLuint captureFBO = 0,
        GLuint captureRBO = 0);

    // Destrói GL objects (se você quiser liberar explicitamente)
    static void Destroy(IBLSet& set);

    // === Pipeline de geração ===
    static GLuint LoadHDRTexture(const std::string& hdrPath);
private:

    static GLuint CreateCubemap_Tex(int size);
    static void   RenderHDRToCubemap(GLuint hdrTexture, GLuint envCubemap,
        Shader& shEquirect, const glm::mat4& proj,
        const std::vector<glm::mat4>& views,
        GLuint captureFBO, GLuint captureRBO);

    static GLuint CreateIrradiance_Tex(int size);
    static void   ConvolveIrradianceToDiffuse(GLuint envCubemap, GLuint irradiance,
        Shader& shIrradiance, const glm::mat4& proj,
        const std::vector<glm::mat4>& views,
        GLuint captureFBO, GLuint captureRBO);

    static GLuint CreatePrefilter_Tex(int baseSize, int mips);
    static void   PrefilterToSpecular(GLuint envCubemap, GLuint prefilter,
        Shader& shPrefilter, const glm::mat4& proj,
        const std::vector<glm::mat4>& views,
        GLuint captureFBO, GLuint captureRBO, int maxMip);

    static GLuint CreateBRDFLUT_Tex(int size);
    static void   IntegrateBRDF(GLuint brdfLUT, Shader& shBRDF,
        GLuint captureFBO, GLuint captureRBO, int size);
     

    //Helpers 
    // Cria um IBLSet vazio (para ser preenchido via GPU_Lerp)
public:
    static IBLSet CreateEmptyIBL(int width = 512, int mipLevels = 5);
private:
    static GLuint CreateEmptyCubemap(int width, int mipLevels);
};

// render helpers (internos — implementados em .cpp com o seu renderCube/renderQuad)
//void ibl_renderCube() {  };
//void ibl_renderQuad() {  };








//int mips = static_cast<int>(std::floor(std::log2(baseSize))) + 1;
//std::log2(baseSize) → quantas vezes dá pra dividir por 2 até chegar em 1.
//+1 → inclui o nível 0 (a resolução original).





//class Engine {
//public:
//    static Engine& GetInstance() {
//        static Engine instance; // criado 1x
//        return instance;
//    }
//
//    void Init() { /* ... */ }
//    void Run() { /* ... */ }
//
//private:
//    Engine() {}                        // construtor privado
//    Engine(const Engine&) = delete;    // sem cópia
//    Engine& operator=(const Engine&) = delete;
//};
