#include "IBLManager.h"
#include <stb/stb_image.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include "../Graphics/Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include "../Graphics/FullscreenQuad.h"

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

// === Interface principal ===
//create ambient IBL
IBLSet IBLManager::EnsureIBL(const std::string& hdrPath,
    const std::string& cacheDir,
    Shader& shEquirect,
    Shader& shIrradiance,
    Shader& shPrefilter,
    Shader& shBRDF,
    GLuint captureFBO,
    GLuint captureRBO)
{
    IBLSet set{};
    const auto base = MakeBaseName(cacheDir, hdrPath);

    // tenta carregar do cache
    if (TryLoadFromCache(base, set)) {
        return set;
    }

    // gera tudo
    stbi_set_flip_vertically_on_load(true);
    int w = 0, h = 0;
    GLuint hdr = LoadHDRTexture(hdrPath, w, h);
    if (!hdr) {
        std::cerr << "[IBL] Falha ao carregar HDR: " << hdrPath << "\n";
        return set;
    }

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    const auto proj = CaptureProjection();
    const auto views = CaptureViews();

    set.envCubemap = CreateEnvCubemap(512);
    RenderToEnvCubemap(hdr, set.envCubemap, shEquirect, proj, views, captureFBO, captureRBO);
    glDeleteTextures(1, &hdr);

    set.irradiance = CreateIrradiance(32);
    ConvolveIrradiance(set.envCubemap, set.irradiance, shIrradiance, proj, views, captureFBO, captureRBO);

    set.prefilter = CreatePrefilter(128, 5);
    PrefilterSpecular(set.envCubemap, set.prefilter, shPrefilter, proj, views, captureFBO, captureRBO, 5);

    set.brdfLUT = CreateBRDFLUT(512);
    IntegrateBRDF(set.brdfLUT, shBRDF, captureFBO, captureRBO, 512);

    SaveToCache(base, set);
    return set;
}

void IBLManager::Destroy(IBLSet& s) {
    if (s.envCubemap) glDeleteTextures(1, &s.envCubemap);
    if (s.irradiance) glDeleteTextures(1, &s.irradiance);
    if (s.prefilter)  glDeleteTextures(1, &s.prefilter);
    if (s.brdfLUT)    glDeleteTextures(1, &s.brdfLUT);
    s = {};
}

// === Construção ===
GLuint IBLManager::LoadHDRTexture(const std::string& path, int& w, int& h) {
    int n;
    float* data = stbi_loadf(path.c_str(), &w, &h, &n, 0);
    if (!data) return 0;
    GLuint tex = 0; glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    stbi_image_free(data);
    return tex;
}

GLuint IBLManager::CreateEnvCubemap(int size) {
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

void IBLManager::RenderToEnvCubemap(GLuint hdrTexture, GLuint envCubemap,
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
        RenderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

GLuint IBLManager::CreateIrradiance(int size) {
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

void IBLManager::ConvolveIrradiance(GLuint envCubemap, GLuint irradiance,
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

GLuint IBLManager::CreatePrefilter(int baseSize, int mips) {
    GLuint id; glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, baseSize, baseSize, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    return id;
}

void IBLManager::PrefilterSpecular(GLuint envCubemap, GLuint prefilter,
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

GLuint IBLManager::CreateBRDFLUT(int size) {
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

// === Cache muito simples ===
// Formato .bin (use quando não tiver KTX/DDS):
// header: char[4]="IBL0" + uint32 type (0=CM,1=2D) + uint32 w + uint32 h + uint32 levels + uint32 faces
// data: glGetTexImage por nível/face em RGB16F (ou RG16F p/ LUT). Sem compressão.
static bool ReadBin(const std::string& path, std::vector<char>& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    f.seekg(0, std::ios::end); auto sz = f.tellg(); f.seekg(0);
    out.resize((size_t)sz);
    f.read(out.data(), sz);
    return true;
}
static bool WriteBin(const std::string& path, const std::vector<char>& buf) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f.write(buf.data(), (std::streamsize)buf.size());
    return true;
}

static void SaveCubeRGB16F(const std::string& path, GLuint tex, int baseSize, int levels) {
    std::vector<char> buf;
    auto put = [&](auto v) { char* p = (char*)&v; buf.insert(buf.end(), p, p + sizeof(v)); };

    // header
    put(uint32_t('I' << 24 | 'B' << 16 | 'L' << 8 | '0'));
    put(uint32_t(0)); // type 0=cubemap
    put(uint32_t(baseSize));
    put(uint32_t(baseSize));
    put(uint32_t(levels));
    put(uint32_t(6));

    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    for (int lv = 0; lv < levels; ++lv) {
        int w = std::max(1, baseSize >> lv);
        int h = std::max(1, baseSize >> lv);
        std::vector<float> tmp((size_t)w * h * 3);
        for (int face = 0; face < 6; ++face) {
            glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, lv, GL_RGB, GL_FLOAT, tmp.data());
            char* p = (char*)tmp.data();
            buf.insert(buf.end(), p, p + tmp.size() * sizeof(float));
        }
    }
    WriteBin(path, buf);
}

static GLuint LoadCubeRGB16F(const std::string& path, int& baseSize, int& levels) {
    std::vector<char> buf;
    if (!ReadBin(path, buf)) return 0;
    auto rdU32 = [&](size_t& off) { uint32_t v; memcpy(&v, &buf[off], 4); off += 4; return v; };
    size_t off = 0;
    uint32_t magic = rdU32(off); if (magic != uint32_t('I' << 24 | 'B' << 16 | 'L' << 8 | '0')) return 0;
    uint32_t type = rdU32(off); if (type != 0) return 0;
    baseSize = (int)rdU32(off);
    rdU32(off); // h (igual ao w para cubemap)
    levels = (int)rdU32(off);
    rdU32(off); // faces=6

    GLuint id; glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (int lv = 0; lv < levels; ++lv) {
        int w = std::max(1, baseSize >> lv);
        int h = std::max(1, baseSize >> lv);
        for (int face = 0; face < 6; ++face) {
            size_t count = (size_t)w * h * 3;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, lv, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, &buf[off]);
            off += count * sizeof(float);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return id;
}

static void Save2DRG16F(const std::string& path, GLuint tex, int size) {
    std::vector<char> buf;
    auto put = [&](auto v) { char* p = (char*)&v; buf.insert(buf.end(), p, p + sizeof(v)); };
    put(uint32_t('I' << 24 | 'B' << 16 | 'L' << 8 | '0'));
    put(uint32_t(1)); // type 1 = 2D
    put(uint32_t(size));
    put(uint32_t(size));
    put(uint32_t(1)); // levels
    put(uint32_t(1)); // faces

    glBindTexture(GL_TEXTURE_2D, tex);
    std::vector<float> tmp((size_t)size * size * 2);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, tmp.data());
    char* p = (char*)tmp.data();
    buf.insert(buf.end(), p, p + tmp.size() * sizeof(float));
    WriteBin(path, buf);
}

static GLuint Load2DRG16F(const std::string& path, int& size) {
    std::vector<char> buf;
    if (!ReadBin(path, buf)) return 0;
    auto rdU32 = [&](size_t& off) { uint32_t v; memcpy(&v, &buf[off], 4); off += 4; return v; };
    size_t off = 0;
    uint32_t magic = rdU32(off); if (magic != uint32_t('I' << 24 | 'B' << 16 | 'L' << 8 | '0')) return 0;
    uint32_t type = rdU32(off); if (type != 1) return 0;
    size = (int)rdU32(off);
    rdU32(off); // h
    rdU32(off); // levels
    rdU32(off); // faces

    GLuint id; glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, size, size, 0, GL_RG, GL_FLOAT, &buf[off]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return id;
}

bool IBLManager::TryLoadFromCache(const std::string& base, IBLSet& out) {
    int size = 0, levels = 0;
    if (GLuint env = LoadCubeRGB16F(base + "_env.bin", size, levels)) out.envCubemap = env; else return false;
    if (GLuint irr = LoadCubeRGB16F(base + "_irr.bin", size, levels)) out.irradiance = irr; else return false;
    if (GLuint pre = LoadCubeRGB16F(base + "_pref.bin", size, levels)) out.prefilter = pre; else return false;
    int lutSz = 0;
    if (GLuint lut = Load2DRG16F(base + "_brdf.bin", lutSz)) out.brdfLUT = lut; else return false;
    return true;
}

void IBLManager::SaveToCache(const std::string& base, const IBLSet& s) {
    // assumptions de tamanhos default
    SaveCubeRGB16F(base + "_env.bin", s.envCubemap, 512, 1 + (int)std::floor(std::log2(512)));
    SaveCubeRGB16F(base + "_irr.bin", s.irradiance, 32, 1);
    SaveCubeRGB16F(base + "_pref.bin", s.prefilter, 128, 5);
    Save2DRG16F(base + "_brdf.bin", s.brdfLUT, 512);
}

std::string IBLManager::MakeBaseName(const std::string& cacheDir, const std::string& hdrPath) {
    return cacheDir + "/" + HashPath(hdrPath);
}

std::string IBLManager::HashPath(const std::string& s) {
    // hash bem simples só pra nome: djb2
    unsigned long h = 5381;
    for (unsigned char c : s) h = ((h << 5) + h) + c;
    std::ostringstream oss; oss << std::hex << h;
    return oss.str();
}
