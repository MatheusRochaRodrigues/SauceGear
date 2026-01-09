#pragma once
#include <unordered_map>
#include <string>
#include "Font.h"
#include "FontAtlasBuilder.h"

class FontManager {
public:
    // cria e retorna ID
    static uint32_t Load(const std::string& path, int px) {
        std::string key = path + std::to_string(px);
        if (keyToID.count(key)) return keyToID[key];

        Font* font = FontAtlasBuilder::Build(path, px); 
        if (font == nullptr) return 0; 

        uint32_t id = nextID++;
        fonts[id] = font;
        keyToID[key] = id;
        return id;
    }

    static Font* Get(uint32_t id) {
        auto it = fonts.find(id);
        return (it != fonts.end()) ? it->second : nullptr;
    }

private:
    static inline uint32_t nextID = 0;  // 1
    static inline std::unordered_map<uint32_t, Font*> fonts;
    static inline std::unordered_map<std::string, uint32_t> keyToID; 
};
