#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "IBLSet.h"

// Encapsula cubemaps e LUT gerados a partir de um HDR e faz cache em disco.

class Shader;

//IBLManager (gera / carrega / salva IBL)
class IBLManager {
public:
    // Gera ou carrega do cache.
    // cacheDir: Resources/Cache/IBL/
    // Retorna IBLSet preenchido.
    static IBLSet EnsureIBL(const std::string& hdrPath, const std::string& cacheDir, bool isAlreadyCubeMap = false);

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

    static GLuint CreateCubemap(int size, GLenum internalFormat, int mipLevels = 1);

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
