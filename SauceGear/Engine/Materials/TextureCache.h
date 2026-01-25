#pragma once
#include "../Graphics/Texture.h"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include <sstream>
#include <iomanip>

// Simple helper to create a string key for a color/float
static uint32_t PackColor(const glm::vec4& c) {
    uint32_t r = uint32_t(glm::clamp(c.r, 0.f, 1.f) * 255.f);
    uint32_t g = uint32_t(glm::clamp(c.g, 0.f, 1.f) * 255.f);
    uint32_t b = uint32_t(glm::clamp(c.b, 0.f, 1.f) * 255.f);
    uint32_t a = uint32_t(glm::clamp(c.a, 0.f, 1.f) * 255.f);
    return (a << 24) | (b << 16) | (g << 8) | r;
}
 
// Simple helper to create a string key for a color/float
inline std::string ColorKey(const glm::vec4& c) {
    std::ostringstream ss;
    ss << std::setw(3) << (int)std::round(c.r * 255.0f) << "_"
        << std::setw(3) << (int)std::round(c.g * 255.0f) << "_"
        << std::setw(3) << (int)std::round(c.b * 255.0f) << "_"
        << std::setw(3) << (int)std::round(c.a * 255.0f);
    return ss.str();
}
inline std::string FloatKey(float v) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3) << v;
    return ss.str();
}

class TextureCache {
public:
    static TextureCache& Get() {
        static TextureCache inst;
        return inst;
    }

    std::shared_ptr<Texture> GetSolidColor(const glm::vec4& color) {
        uint32_t key = PackColor(color);

        std::lock_guard<std::mutex> lock(mutex);
        auto it = solidColorCache.find(key);
        if (it != solidColorCache.end())
            return it->second;

        uint8_t px[4] = {
            uint8_t(color.r * 255),
            uint8_t(color.g * 255),
            uint8_t(color.b * 255),
            uint8_t(color.a * 255)
        };

        auto tex = std::make_shared<Texture>();
        tex->CreateFromMemory(px, 1, 1, GL_RGBA);
        tex->isSolidColor = true;
        tex->dataColor = color;

        solidColorCache[key] = tex;
        return tex;
    }



    //String

    // return shared_ptr to a cached 1x1 float as grayscale RGBA
    std::shared_ptr<Texture> GetFloat(float value) {
        auto key = FloatKey(value);
        std::lock_guard<std::mutex> l(mutex);
        auto it = floatCache.find(key);
        if (it != floatCache.end()) return it->second;
        uint8_t vv = static_cast<uint8_t>(glm::clamp(value, 0.0f, 1.0f) * 255.0f);
        uint8_t px[4] = { vv, vv, vv, 255 };

        auto tex = std::make_shared<Texture>();
        tex->CreateFromMemory(px, 1, 1, GL_RGBA);
        floatCache[key] = tex;
        return tex;
    }


    std::shared_ptr<Texture> Load(const std::string& filename, bool isSRGB = false)
    {
        auto it = texCache.find(filename);
        if (it != texCache.end()) {
            // já existe
            auto tex = it->second;
            return tex;
        }
        else {
            // não existe, cria e insere
            texCache[filename] = make_shared<Texture>();
            texCache[filename]->LoadFromFile(filename, isSRGB);
            return texCache[filename];
        }
    }

    //Seu TextureCache poderia ser implementado em cima do AssetDatabase genérico
    /*std::shared_ptr<Texture> Load(const std::string& filename) {
        auto tex = AssetDatabase::Load<Texture>(filename);
        if (!tex->IsLoaded())
            tex->LoadFromFile(filename);
        return tex;
    }*/
    //Ou seja, TextureCache vira só um wrapper de conveniência em cima do AssetDatabase.

private:
    std::unordered_map<uint32_t, std::shared_ptr<Texture>> solidColorCache;
    std::mutex mutex;


    TextureCache() = default;
    std::unordered_map<std::string, std::shared_ptr<Texture>> texCache;
    std::unordered_map<std::string, std::shared_ptr<Texture>> colorCache;
    std::unordered_map<std::string, std::shared_ptr<Texture>> floatCache;
};
