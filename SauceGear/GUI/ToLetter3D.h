#pragma once

#include <map>
#include <string>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../Graphics/Shader.h"
#include "../Core/Camera.h"

struct Character3D {
    unsigned int TextureID;
    glm::ivec2   Size;
    glm::ivec2   Bearing;
    unsigned int Advance;
};

class TextRenderer3D {
public:
    std::map<char, Character3D> Characters;
    unsigned int VAO, VBO;
    Shader* shader;

    TextRenderer3D() {
        shader = new Shader( "GUI/SimpleText3D.vs",  "GUI/SimpleText3D.fs" );

        std::string fontPath = ("Assets/Fonts/Roboto-Medium.ttf");
        LoadFont(fontPath);
        InitRenderData();
    }

    void RenderText3D(
        Camera* cam,
        const std::string& text,
        const glm::vec3& worldPos,
        float scale,
        const glm::vec3& color,
        bool billboard
    ) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);

        shader->use();
        glUniform3fv(
            glGetUniformLocation(shader->ID, "textColor"),
            1, glm::value_ptr(color)
        );

        glm::mat4 model(1.0f);
        model = glm::translate(model, worldPos);

        if (billboard) {
            model[0] = glm::vec4(cam->Right * scale, 0.0f);
            model[1] = glm::vec4(cam->Up * scale, 0.0f);
            model[2] = glm::vec4(cam->Front * scale, 0.0f);
        }
        else {
            model = glm::scale(model, glm::vec3(scale));
        }

        glm::mat4 mvp =
            cam->GetProjectionMatrix() *
            cam->GetViewMatrix() *
            model;

        glUniformMatrix4fv(
            glGetUniformLocation(shader->ID, "MVP"),
            1, GL_FALSE, glm::value_ptr(mvp)
        );

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        float x = 0.0f;
        float y = 0.0f;

        for (char c : text) {
            Character3D ch = Characters[c];

            float xpos = x + ch.Bearing.x;
            float ypos = y - (ch.Size.y - ch.Bearing.y);

            float w = ch.Size.x;
            float h = ch.Size.y;

            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };

            glBindTexture(GL_TEXTURE_2D, ch.TextureID);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

            glEnable(GL_DEPTH_TEST);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            glDisable(GL_DEPTH_TEST);

            x += (ch.Advance >> 6);
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_BLEND);
    }

private:
    void LoadFont(const std::string& path) {
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            std::cout << "FreeType init failed\n";
            return;
        }

        FT_Face face;
        if (FT_New_Face(ft, path.c_str(), 0, &face)) {
            std::cout << "Font load failed\n";
            return;
        }

        FT_Set_Pixel_Sizes(face, 0, 48);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
                continue;

            unsigned int tex;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Characters[c] = {
                tex,
                { face->glyph->bitmap.width, face->glyph->bitmap.rows },
                { face->glyph->bitmap_left, face->glyph->bitmap_top },
                (unsigned int)face->glyph->advance.x
            };
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    void InitRenderData() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 4, GL_FLOAT, GL_FALSE,
            4 * sizeof(float),
            (void*)0
        );

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};
