#include <stb/stb_image.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath> 
#include <vector>  
#include <glad/glad.h> 
 
class IBLPersistence {
public:
    // === Cache binário simples ===
    // cada .bin tem um header leve e, para cubemap, grava todas faces/mips em sequência. 
    bool TryLoadFromCache(const std::string& base, IBLSet& out) {
        int size = 0, levels = 0;
        if (GLuint env = LoadCubeRGB16F(base + "_env.bin", size, levels)) out.envCubemap = env; else return false;
        if (GLuint irr = LoadCubeRGB16F(base + "_irr.bin", size, levels)) out.irradiance = irr; else return false;
        if (GLuint pre = LoadCubeRGB16F(base + "_pref.bin", size, levels)) out.prefilter = pre; else return false;
        int lutSz = 0;
        if (GLuint lut = Load2DRG16F(base + "_brdf.bin", lutSz)) out.brdfLUT = lut; else return false;
        return true;
    }

    void SaveToCache(const std::string& base, const IBLSet& s) {
        // assumptions de tamanhos default
        SaveCubeRGB16F(base + "_env.bin", s.envCubemap, 512, 1 + (int)std::floor(std::log2(512)));
        SaveCubeRGB16F(base + "_irr.bin", s.irradiance, 32, 1);
        SaveCubeRGB16F(base + "_pref.bin", s.prefilter, 128, 5);
        Save2DRG16F(base + "_brdf.bin", s.brdfLUT, 512);
    }

    // utilitários
    std::string MakeBaseName(const std::string& cacheDir, const std::string& hdrPath) {
        return cacheDir + "/" + HashPath(hdrPath);
    }

    // utilitários
    std::string HashPath(const std::string& s) {        // hash leve p/ nomear arquivos
        // hash bem simples só pra nome: djb2
        unsigned long h = 5381;
        for (unsigned char c : s) h = ((h << 5) + h) + c;
        std::ostringstream oss; oss << std::hex << h;
        return oss.str();
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


};
 