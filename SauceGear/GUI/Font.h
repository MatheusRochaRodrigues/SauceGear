#pragma once
#include <glm/glm.hpp>
#include <string>
#include <glad/glad.h>
#include <unordered_map>

/// Holds all state information relevant to a character as loaded using FreeType
struct Glyph {                  // Glyph == one Character
    glm::ivec2 size;            // Size of glyph
    glm::ivec2 bearing;         // Offset from baseline to left/top of glyph
    uint32_t advance;           // Horizontal offset to advance to next glyph

    glm::vec2 uvMin;            // map to Atlas
    glm::vec2 uvMax;            // map to Atlas
};
 
class Font {
public:
    GLuint atlasTexture = 0;
    int pixelSize = 32;
    int atlasSize = 1024;
    std::unordered_map<char, Glyph> glyphs;
};

 
