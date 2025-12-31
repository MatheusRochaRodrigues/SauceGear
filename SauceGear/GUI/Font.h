#pragma once
#include <glm/glm.hpp>
#include <string>
#include <glad/glad.h>
#include <unordered_map>

struct Glyph {
    glm::ivec2 size;
    glm::ivec2 bearing;
    uint32_t advance;

    glm::vec2 uvMin;
    glm::vec2 uvMax;
};
 
class Font {
public:
    GLuint atlasTexture = 0;
    int pixelSize = 32;
    int atlasSize = 1024;
    std::unordered_map<char, Glyph> glyphs;
};

 
