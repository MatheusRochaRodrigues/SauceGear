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
        GLuint captureFBO,
        GLuint captureRBO);

    // Destrói GL objects (se você quiser liberar explicitamente)
    static void Destroy(IBLSet& set);

private:
    // === Pipeline de geração ===
    static GLuint LoadHDRTexture(const std::string& hdrPath, int& w, int& h);
    static GLuint CreateEnvCubemap(int size);
    static void   RenderToEnvCubemap(GLuint hdrTexture, GLuint envCubemap,
        Shader& shEquirect, const glm::mat4& proj,
        const std::vector<glm::mat4>& views,
        GLuint captureFBO, GLuint captureRBO);

    static GLuint CreateIrradiance(int size);
    static void   ConvolveIrradiance(GLuint envCubemap, GLuint irradiance,
        Shader& shIrradiance, const glm::mat4& proj,
        const std::vector<glm::mat4>& views,
        GLuint captureFBO, GLuint captureRBO);

    static GLuint CreatePrefilter(int baseSize, int mips);
    static void   PrefilterSpecular(GLuint envCubemap, GLuint prefilter,
        Shader& shPrefilter, const glm::mat4& proj,
        const std::vector<glm::mat4>& views,
        GLuint captureFBO, GLuint captureRBO, int maxMip);

    static GLuint CreateBRDFLUT(int size);
    static void   IntegrateBRDF(GLuint brdfLUT, Shader& shBRDF,
        GLuint captureFBO, GLuint captureRBO, int size);


    // === Cache binário simples ===
    // cada .bin tem um header leve e, para cubemap, grava todas faces/mips em sequência.
    static bool   TryLoadFromCache(const std::string& base, IBLSet& out);
    static void   SaveToCache(const std::string& base, const IBLSet& set);

    // utilitários
    static std::string MakeBaseName(const std::string& cacheDir, const std::string& hdrPath);
    static std::string HashPath(const std::string& s); // hash leve p/ nomear arquivos
};

// render helpers (internos — implementados em .cpp com o seu renderCube/renderQuad)
//void ibl_renderCube() {  };
//void ibl_renderQuad() {  };
