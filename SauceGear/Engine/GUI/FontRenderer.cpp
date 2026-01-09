#include "FontRenderer.h"
#include "../Graphics/Shader.h"
#include <glad/glad.h>

GLuint FontRenderer::vao;
GLuint FontRenderer::vboQuad;
GLuint FontRenderer::vboInstance;
Shader* FontRenderer::shader = nullptr;

void FontRenderer::SetShader(Shader* s) { shader = s; } 
void FontRenderer::Init() {  
    float quad[] = {
        0,0, 0,0,
        1,0, 1,0,
        1,1, 1,1,

        0,0, 0,0,
        1,1, 1,1,
        0,1, 0,1
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
    glBufferData( GL_ARRAY_BUFFER, 10000 * sizeof(GlyphInstance), nullptr, GL_DYNAMIC_DRAW );

    size_t stride = sizeof(GlyphInstance); 
    auto Attr = [&](int loc, int count, size_t offset) {
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(
            loc, count, GL_FLOAT, GL_FALSE,
            stride, (void*)offset
        );
        glVertexAttribDivisor(loc, 1);
    };

    Attr(1, 3, offsetof(GlyphInstance, Anchor));
    Attr(2, 2, offsetof(GlyphInstance, Offset));
    Attr(3, 2, offsetof(GlyphInstance, Size));
    Attr(4, 4, offsetof(GlyphInstance, UV_MM));
    Attr(5, 4, offsetof(GlyphInstance, Color));
    Attr(6, 4, offsetof(GlyphInstance, Outline_ThicknessRGB));
    Attr(7, 4, offsetof(GlyphInstance, ShadowA));
    Attr(8, 2, offsetof(GlyphInstance, ShadowB)); 
}

void FontRenderer::Begin() {  for (auto& [_, b] : batches) b.clear();  }   
void FontRenderer::Submit(Font* f, const GlyphInstance& g) { batches[f].push_back( g); } 

void FontRenderer::Flush(const glm::mat4& mvp, bool depthTest) {
    if (depthTest) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    shader->use();
    shader->setInt("uMode", 0);// 2D
    shader->setMat4("M_VP", mvp); 
    glActiveTexture(GL_TEXTURE0);
    shader->setInt("uAtlas", 0);

    glBindVertexArray(vao);
     
    for (auto& [font, batch] : batches) {
        //printf("Draw %zu glyphs \n", batch.size()); 
        if (batch.empty()) continue;

        glBindTexture(GL_TEXTURE_2D, font->atlasTexture);
        glBindBuffer(GL_ARRAY_BUFFER, vboInstance);

        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            batch.size() * sizeof(GlyphInstance),
            batch.data()
        );  

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, batch.size());
    } 

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}


void FontRenderer::Flush3D(
    const glm::mat4& VP,
    const glm::vec3& camRight,
    const glm::vec3& camUp,
    bool depthTest
) {
    if (depthTest) glEnable(GL_DEPTH_TEST);
    else           glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    shader->use();

    shader->setMat4("M_VP", VP);
    shader->setVec3("uCamRight", camRight);
    shader->setVec3("uCamUp", camUp);
    shader->setInt("uMode", 1); // 🔥 3D BILLBOARD

    glActiveTexture(GL_TEXTURE0);
    shader->setInt("uAtlas", 0);

    glBindVertexArray(vao);

    for (auto& [font, batch] : batches) {
        if (batch.empty()) continue;

        glBindTexture(GL_TEXTURE_2D, font->atlasTexture);
        glBindBuffer(GL_ARRAY_BUFFER, vboInstance);

        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            batch.size() * sizeof(GlyphInstance),
            batch.data()
        );

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, batch.size());
    }

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}








/* Init() ==
 
    //int loc = 1;
    //size_t offset = 0;

    //auto Attr = [&](int count) {
    //    glEnableVertexAttribArray(loc);
    //    glVertexAttribPointer(
    //        loc, count, GL_FLOAT, GL_FALSE,
    //        sizeof(GlyphInstance),
    //        (void*)offset
    //    );
    //    glVertexAttribDivisor(loc, 1);
    //    offset += count * sizeof(float);
    //    loc++;
    //};

    //Attr(4); // pos.xy | size.xy
    //Attr(4); // uvMin.xy | uvMax.xy
    //Attr(4); // color
    //Attr(4); // outlineThickness | outlineColor.rgb
    //Attr(4); // shadowOffset.xy | shadowColor.rg
    //Attr(2); // shadowColor.ba

    // SECOND WAY 
    for (int i = 0; i < 5; i++) {
        glEnableVertexAttribArray(1 + i);
        glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(GlyphInstance), (void*)(i * 16));
        glVertexAttribDivisor(1 + i, 1);
    } 

*/






//std::unordered_map<Font*, std::vector<GlyphInstance>> FontRenderer::batches;
// 
//static GlyphInstanceGPU ToGPU(const GlyphInstance& g) {
//    GlyphInstanceGPU o{};
//
//    o.posSize[0] = g.pos.x;
//    o.posSize[1] = g.pos.y;
//    o.posSize[2] = g.size.x;
//    o.posSize[3] = g.size.y;
//
//    o.uv[0] = g.uvMin.x;
//    o.uv[1] = g.uvMin.y;
//    o.uv[2] = g.uvMax.x;
//    o.uv[3] = g.uvMax.y;
//
//    memcpy(o.color, &g.color[0], sizeof(float) * 4);
//
//    o.outline[0] = g.outlineThickness;
//    o.outline[1] = g.outlineColor.r;
//    o.outline[2] = g.outlineColor.g;
//    o.outline[3] = g.outlineColor.b;
//
//    o.shadowA[0] = g.shadowOffset.x;
//    o.shadowA[1] = g.shadowOffset.y;
//    o.shadowA[2] = g.shadowColor.r;
//    o.shadowA[3] = g.shadowColor.g;
//
//    o.shadowB[0] = g.shadowColor.b;
//    o.shadowB[1] = g.shadowColor.a;
//
//    return o;
//}