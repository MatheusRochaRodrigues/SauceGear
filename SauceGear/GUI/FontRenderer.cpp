#include "FontRenderer.h"
#include "../Graphics/Shader.h"
#include <glad/glad.h>

GLuint FontRenderer::vao;
GLuint FontRenderer::vboQuad;
GLuint FontRenderer::vboInstance;
std::unordered_map<Font*, TextBatch> FontRenderer::batches;
Shader* FontRenderer::shader = nullptr;

void FontRenderer::SetShader(Shader* s) { shader = s; }

void FontRenderer::Init() {
    float quad[] = {
        0,1, 0,1,
        1,0, 1,0,
        0,0, 0,0,
        0,1, 0,1,
        1,1, 1,1,
        1,0, 1,0
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboQuad);
    glBindBuffer(GL_ARRAY_BUFFER, vboQuad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    
    glGenBuffers(1, &vboInstance);
    glBindBuffer(GL_ARRAY_BUFFER, vboInstance);
    glBufferData(GL_ARRAY_BUFFER, 10000 * sizeof(GlyphInstance), nullptr, GL_DYNAMIC_DRAW);

    // ---- INSTANCE ATTRIBUTES ----
    // layout:
    // 1 vec4 = pos.xy | size.xy
    // 2 vec4 = uvMin.xy | uvMax.xy
    // 3 vec4 = color
    // 4 vec4 = outlineThickness | outlineColor.rgb
    // 5 vec4 = shadowOffset.xy | shadowColor.rg
    // 6 vec4 = shadowColor.ba

    int loc = 1;
    size_t offset = 0;

    auto Attr = [&](int count) {
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(
            loc, count, GL_FLOAT, GL_FALSE,
            sizeof(GlyphInstance),
            (void*)offset
        );
        glVertexAttribDivisor(loc, 1);
        offset += count * sizeof(float);        
        loc++;
    };

    Attr(4); // pos.xy | size.xy
    Attr(4); // uvMin.xy | uvMax.xy
    Attr(4); // color
    Attr(4); // outlineThickness | outlineColor.rgb
    Attr(4); // shadowOffset.xy | shadowColor.rg
    Attr(2); // shadowColor.ba



    // SECOND WAY
    /* 
    for (int i = 0; i < 5; i++) {
        glEnableVertexAttribArray(1 + i);
        glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(GlyphInstance), (void*)(i * 16));
        glVertexAttribDivisor(1 + i, 1);
    }*/
}

void FontRenderer::Begin() {
    for (auto& [_, b] : batches) b.instances.clear();
}

void FontRenderer::Submit(Font* f, const GlyphInstance& g) {
    batches[f].font = f;
    batches[f].instances.push_back(g);
}

void FontRenderer::Flush(const glm::mat4& mvp, bool depthTest) {
    if (depthTest) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);

    shader->use();
    shader->setMat4("MVP", mvp);

    glActiveTexture(GL_TEXTURE0);
    shader->setInt("uMSDF", 0);

    glBindVertexArray(vao);

    for (auto& [font, batch] : batches) {
        if (batch.instances.empty()) continue;

        glBindTexture(GL_TEXTURE_2D, font->atlasTexture);
        glBindBuffer(GL_ARRAY_BUFFER, vboInstance);

        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            batch.instances.size() * sizeof(GlyphInstance),
            batch.instances.data()
        );

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, batch.instances.size());
    }
}
