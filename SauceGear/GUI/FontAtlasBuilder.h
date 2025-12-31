#pragma once 
#include "Font.h"
#include <string>

class FontAtlasBuilder {
public:
    static Font* Build(const std::string& path, int pixelSize);
};
