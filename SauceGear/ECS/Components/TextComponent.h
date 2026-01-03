#pragma once
#include <string>
#include <glm/glm.hpp>
#include <cstdint>
#include "../GUI/FontRenderer.h"
 
struct TextStyle {
    float outlineThickness = 0.0f;
    glm::vec4 outlineColor = { 0,0,0,1 };       

    glm::vec2 shadowOffset = { 0,0 };
    glm::vec4 shadowColor = { 0,0,0,0 };
};

struct TextLayoutCache {
    std::vector<GlyphInstance> glyphs;
    float width = 0.0f;
    float height = 0.0f;
};

struct TextComponent {
    std::string text;

    uint32_t fontID = 0;    // qual fonte
    glm::vec4 color = { 1,1,1,1 };
    float scale = 1.0f;

    bool dirty = true;      // só regenera se mudar

    enum class Space {
        Screen,   // GUI
        World     // 3D
    } space = Space::Screen;

    enum class Units {
        Pixels,   // posição em pixels
        Relative  // 0..1 relativo à tela
    } units = Units::Pixels;

    enum class Align {
        Left,
        Center,
        Right
    } align = Align::Left;

    enum class Anchor {
        TopLeft,
        TopCenter,
        TopRight,
        CenterLeft,
        Center,
        CenterRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    } anchor = Anchor::TopLeft;
     
    bool billboard = true;    // world text
    bool depthTest = false;   // world text

    TextLayoutCache layout;
    TextStyle style;

    void LoadFont(string s) {

        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }
};

