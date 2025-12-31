#include "FontAtlasBuilder.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <vector>
#include <cmath>
#include <iostream>

static float sdf(const unsigned char* bitmap, int w, int h, int x, int y) {
    float minDist = 1e9f;
    bool inside = bitmap[y * w + x] > 128;

    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++) {
            bool in = bitmap[j * w + i] > 128;
            if (in != inside) {
                float dx = i - x, dy = j - y;
                minDist = std::min(minDist, dx * dx + dy * dy);
            }
        }

    minDist = std::sqrt(minDist);
    return inside ? 0.5f + minDist * 0.01f : 0.5f - minDist * 0.01f;
}

Font* FontAtlasBuilder::Build(const std::string& path, int pixelSize) {
    FT_Library ft; 

    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library ---- " << path.c_str() << std::endl;
        return nullptr;
    }

    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font ---- " << path.c_str() << std::endl;
        return nullptr;
    }

    FT_Set_Pixel_Sizes(face, 0, pixelSize);

    Font* font = new Font();
    font->pixelSize = pixelSize;

    const int atlasSize = font->atlasSize;
    std::vector<unsigned char> atlas(atlasSize * atlasSize, 0);

    int penX = 0, penY = 0, rowH = 0;
     
    //nao é necessario ensse caso pois é 1 atlas e nao uma textura de um unic glyph
    // disable byte-alignment restriction
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (char c = 32; c < 127; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph (" << c << ")" << " ---- " << path.c_str() << std::endl;
            continue; 
        }

        auto& bmp = face->glyph->bitmap;

        if (penX + bmp.width >= atlasSize) {
            penX = 0;
            penY += rowH;
            rowH = 0;
        }

        for (int y = 0; y < bmp.rows; y++)
            for (int x = 0; x < bmp.width; x++) {
                atlas[(penY + y) * atlasSize + penX + x] =
                    bmp.buffer[y * bmp.pitch + x];
            }

        Glyph g;
        g.size = { bmp.width, bmp.rows };
        g.bearing = {
            face->glyph->bitmap_left,
            face->glyph->bitmap_top
        };
        g.advance = face->glyph->advance.x;

        g.uvMin = {
            (float)penX / atlasSize,
            (float)penY / atlasSize
        };
        g.uvMax = {
            (float)(penX + bmp.width) / atlasSize,
            (float)(penY + bmp.rows) / atlasSize
        };

        font->glyphs[c] = g;

        penX += bmp.width + 2;
        rowH = std::max(rowH, (int)bmp.rows);
    }

    glGenTextures(1, &font->atlasTexture);
    glBindTexture(GL_TEXTURE_2D, font->atlasTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
        atlasSize, atlasSize, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return font;
}
