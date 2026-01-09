#include "FontAtlasBuilder.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <vector>
#include <cmath>
#include <iostream> 
 

static float ComputeSDF( const unsigned char* bmp, int w, int h, int x, int y) {
    const bool inside = bmp[y * w + x] > 128;
    float minDist = 1e9f;

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            bool edge = (bmp[j * w + i] > 128) != inside;
            if (edge) {
                float dx = float(i - x);
                float dy = float(j - y);
                minDist = std::min(minDist, dx * dx + dy * dy);
            }
        }
    }

    minDist = std::sqrt(minDist);

    // normaliza para 0..1 (faixa segura)
    float sdf = 0.5f + (inside ? minDist : -minDist) / 32.0f;
    return glm::clamp(sdf, 0.0f, 1.0f);
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
                float d = ComputeSDF( bmp.buffer, bmp.width, bmp.rows, x, y);
                atlas[(penY + y) * atlasSize + penX + x] = (unsigned char)(d * 255.0f);
            }
         
        Glyph g;
        g.size = { bmp.width, bmp.rows };
        g.bearing = {
            face->glyph->bitmap_left,
            face->glyph->bitmap_top
        };

        g.advance = face->glyph->advance.x;
        //g.advance = face->glyph->advance.x >> 6; testar

        //colocando nas cordenadas UV
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
     
    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
     
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasSize, atlasSize, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); 

    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return font;
}









//noa encessario
/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);*/