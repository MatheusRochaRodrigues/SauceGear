#pragma once 
#include <glm/glm.hpp>

enum class ColorSpace {
    Linear,
    SRGB,
    HDR
};

struct Color {
    glm::vec4 value;
    //float r, g, b, a;
    ColorSpace space = ColorSpace::Linear;

    Color(float r = 1, float g = 1, float b = 1, float a = 1)
        //: r(r), g(g), b(b), a(a) 
    {
        value = glm::vec4(r,g,b,a);
    }

    Color(const glm::vec3& v) /*: r(v.x), g(v.y), b(v.z), a(1.0f)*/ {
        value = glm::vec4(v, 1);
    }
    Color(const glm::vec4& v) /*: r(v.x), g(v.y), b(v.z), a(v.w)*/ {
        value = v;
    }

    operator glm::vec3() const { return  glm::vec3(value.x, value.y, value.z) /*{ r, g, b }*/; }
    operator glm::vec4() const { return value/*{ r, g, b, a }*/; }

    static Color HDR(float r, float g, float b) {
        return { r, g, b, 1.0f };
    }

    static Color HDR(glm::vec3 v) {
        return { v };
    }
};



/*
Color albedo = Color(1, 0, 0);
Color emissive = Color::HDR(5.0f, 2.0f, 1.0f);


*/