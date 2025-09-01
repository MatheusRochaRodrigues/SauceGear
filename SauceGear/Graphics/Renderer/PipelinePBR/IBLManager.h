#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

// Encapsula cubemaps e LUT gerados a partir de um HDR e faz cache em disco.
struct IBLSet {
    GLuint envCubemap   = 0; // 512x512 RGB16F + mipmap
    GLuint irradiance   = 0; // 32x32  RGB16F
    GLuint prefilter    = 0; // 128x128 RGB16F + mips
    GLuint brdfLUT      = 0; // 512x512 RG16F 2D
    bool   valid() const { return envCubemap && irradiance && prefilter && brdfLUT; }

    GLuint debugFace = 0;
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
     
};

// render helpers (internos — implementados em .cpp com o seu renderCube/renderQuad)
//void ibl_renderCube() {  };
//void ibl_renderQuad() {  };















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
