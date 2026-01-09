#pragma once 
#include "../../Graphics/Texture.h"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map> 

inline std::shared_ptr<Texture> resolveTexture(const glm::vec4& col) {
    uint8_t px[4];
    px[0] = static_cast<uint8_t>(glm::clamp(col.r, 0.0f, 1.0f) * 255.0f);
    px[1] = static_cast<uint8_t>(glm::clamp(col.g, 0.0f, 1.0f) * 255.0f);
    px[2] = static_cast<uint8_t>(glm::clamp(col.b, 0.0f, 1.0f) * 255.0f);
    px[3] = static_cast<uint8_t>(glm::clamp(col.a, 0.0f, 1.0f) * 255.0f); 
    auto t = std::make_shared<Texture>();
    t->CreateFromMemory(px, 1, 1, GL_RGBA); // adapta à tua API
    return t;
}

inline std::shared_ptr<Texture> resolveTexture(float v) {
    uint8_t px[4];
    uint8_t vv = static_cast<uint8_t>(glm::clamp(v, 0.0f, 1.0f) * 255.0f);
    px[0] = vv; px[1] = vv; px[2] = vv; px[3] = 255;
    auto t = std::make_shared<Texture>();
    t->CreateFromMemory(px, 1, 1, GL_RGBA);
    return t;
}

// opcional: cache global para não regenerar sempre a mesma cor/valor
inline std::string ColorKey(const glm::vec3& c) {
    return std::to_string((int)(c.r * 255)) + "_" + std::to_string((int)(c.g * 255)) + "_" + std::to_string((int)(c.b * 255));
}
