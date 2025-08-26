#include "IBLManager.h"   
#include "../Graphics/Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include "../Graphics/FullscreenQuad.h"
#include "PersistenceIBL.hpp"

// === Helpers para captura ===
static glm::mat4 CaptureProjection() {
    return glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
}
static std::vector<glm::mat4> CaptureViews() {
    return {
        glm::lookAt(glm::vec3(0,0,0), glm::vec3(1, 0, 0), glm::vec3(0,-1, 0)),
        glm::lookAt(glm::vec3(0,0,0), glm::vec3(-1,0, 0), glm::vec3(0,-1, 0)),
        glm::lookAt(glm::vec3(0,0,0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
        glm::lookAt(glm::vec3(0,0,0), glm::vec3(0,-1, 0), glm::vec3(0, 0,-1)),
        glm::lookAt(glm::vec3(0,0,0), glm::vec3(0, 0, 1), glm::vec3(0,-1, 0)),
        glm::lookAt(glm::vec3(0,0,0), glm::vec3(0, 0,-1), glm::vec3(0,-1, 0)),
    };
}

// === Construção ===
GLuint IBLManager::LoadHDRTexture(const std::string& path) {
    // pbr: load the HDR environment map
    // ---------------------------------
    stbi_set_flip_vertically_on_load(true);

    int width, height, nrComponents;
    float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
    if (!data) return 0;
    GLuint tex = 0; glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    stbi_image_free(data);
    return tex;
}

// === Interface principal ===
//create ambient IBL                    LoadOrBuild
IBLSet IBLManager::EnsureIBL(const std::string& hdrPath,
    const std::string& cacheDir,
    Shader& hdrToCube,
    Shader& Irradiance,
    Shader& Prefilter,
    Shader& BRDF,
    GLuint captureFBO,
    GLuint captureRBO)
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST); // às vezes depth atrapalha

    IBLSet set{};  
    GLuint hdr = LoadHDRTexture(hdrPath);
    if (!hdr) {
        std::cerr << "[IBL] Falha ao carregar HDR: " << hdrPath << "\n";
        return set;
    }

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    const auto proj = CaptureProjection();
    const auto views = CaptureViews();

    set.envCubemap = CreateCubemap_Tex(512);
    RenderHDRToCubemap(hdr, set.envCubemap, hdrToCube, proj, views, captureFBO, captureRBO);
    glDeleteTextures(1, &hdr);

    set.irradiance = CreateIrradiance_Tex(32);
    ConvolveIrradianceToDiffuse(set.envCubemap, set.irradiance, Irradiance, proj, views, captureFBO, captureRBO);

    set.prefilter = CreatePrefilter_Tex(128, 5);
    PrefilterToSpecular(set.envCubemap, set.prefilter, Prefilter, proj, views, captureFBO, captureRBO, 5);

    set.brdfLUT = CreateBRDFLUT_Tex(512);
    IntegrateBRDF(set.brdfLUT, BRDF, captureFBO, captureRBO, 512);
     

    //Debug
     
    int size = 32;
    glGenTextures(1, &set.debugFace);
    glBindTexture(GL_TEXTURE_2D, set.debugFace);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);

    // copia os pixels da face do cubemap para a textura 2D
    glCopyImageSubData(set.irradiance, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1, 0, 0, 0, 0,
        set.debugFace, GL_TEXTURE_2D, 0, 0, 0, 0,
        size, size, 1);

    return set;
}

void IBLManager::Destroy(IBLSet& s) {
    if (s.envCubemap) glDeleteTextures(1, &s.envCubemap);
    if (s.irradiance) glDeleteTextures(1, &s.irradiance);
    if (s.prefilter)  glDeleteTextures(1, &s.prefilter);
    if (s.brdfLUT)    glDeleteTextures(1, &s.brdfLUT);
    s = {};
}


GLuint IBLManager::CreateCubemap_Tex(int size) {
    GLuint id; glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return id;
}

void IBLManager::RenderHDRToCubemap(GLuint hdrTexture, GLuint envCubemap,
    Shader& shEquirect, const glm::mat4& proj,
    const std::vector<glm::mat4>& views,
    GLuint fbo, GLuint rbo) {

    shEquirect.use();
    shEquirect.setInt("equirectangularMap", 0);
    shEquirect.setMat4("projection", proj);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);

    for (int i = 0; i < 6; i++) {
        shEquirect.setMat4("view", views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "FBO incompleto!\n";
        }

        RenderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

GLuint IBLManager::CreateIrradiance_Tex(int size) {
    GLuint id; glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return id;
}

void IBLManager::ConvolveIrradianceToDiffuse(GLuint envCubemap, GLuint irradiance,
    Shader& shIrr, const glm::mat4& proj,
    const std::vector<glm::mat4>& views,
    GLuint fbo, GLuint rbo) {
    shIrr.use();
    shIrr.setInt("environmentMap", 0);
    shIrr.setMat4("projection", proj);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
    for (int i = 0; i < 6; i++) {
        shIrr.setMat4("view", views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint IBLManager::CreatePrefilter_Tex(int baseSize, int mips) {
    GLuint id; glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, baseSize, baseSize, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    return id;
}

void IBLManager::PrefilterToSpecular(GLuint envCubemap, GLuint prefilter,
    Shader& shPref, const glm::mat4& proj,
    const std::vector<glm::mat4>& views,
    GLuint fbo, GLuint rbo, int maxMip) {
    shPref.use();
    shPref.setInt("environmentMap", 0);
    shPref.setMat4("projection", proj);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    for (int mip = 0; mip < maxMip; ++mip) {
        int mipW = (int)(128 * std::pow(0.5, mip));
        int mipH = (int)(128 * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipW, mipH);
        glViewport(0, 0, mipW, mipH);

        float roughness = (float)mip / (float)(maxMip - 1);
        shPref.setFloat("roughness", roughness);
        for (int i = 0; i < 6; i++) {
            shPref.setMat4("view", views[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint IBLManager::CreateBRDFLUT_Tex(int size) {
    GLuint id; glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, size, size, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return id;
}

void IBLManager::IntegrateBRDF(GLuint brdfLUT, Shader& shBRDF,
    GLuint fbo, GLuint rbo, int size) {
    glViewport(0, 0, size, size);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT, 0);

    shBRDF.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderQuad();           // renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
} 