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

    // return shared_ptr to a cached 1x1 color texture
    std::shared_ptr<Texture> GetSolidColor(const glm::vec4& color) {
        auto key = ColorKey(color);
        std::lock_guard<std::mutex> l(mutex);
        auto it = colorCache.find(key);
        if (it != colorCache.end()) return it->second;

        // Create 1x1 RGBA
        uint8_t px[4];
        px[0] = static_cast<uint8_t>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f);
        px[1] = static_cast<uint8_t>(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f);
        px[2] = static_cast<uint8_t>(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f);
        px[3] = static_cast<uint8_t>(glm::clamp(color.a, 0.0f, 1.0f) * 255.0f); 

        auto tex = std::make_shared<Texture>();
        tex->CreateFromMemory(px, 1, 1, GL_RGBA); // ajusta se necessário
        colorCache[key] = tex;
        return tex;
    }

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


    std::shared_ptr<Texture> Load(const std::string& filename)
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
            texCache[filename]->LoadFromFile(filename);
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
    TextureCache() = default;
    std::unordered_map<std::string, std::shared_ptr<Texture>> texCache;
    std::unordered_map<std::string, std::shared_ptr<Texture>> colorCache;
    std::unordered_map<std::string, std::shared_ptr<Texture>> floatCache;
    std::mutex mutex;
};
