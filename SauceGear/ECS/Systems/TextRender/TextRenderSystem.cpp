#include "TextRenderSystem.h"  

static void BuildTextLayout(TextComponent& txt, Font* font)
{
    txt.layout.glyphs.clear();

    float x = 0, y = 0;
    float maxX = 0, minY = 0, maxY = 0;

    for (char c : txt.text)
    {
        if (c == '\n') {
            y += font->pixelSize * txt.scale;
            x = 0;
            continue;
        }

        const Glyph& g = font->glyphs[c];       //to search in Atlas
        GlyphInstance gi{};

        gi.Offset = {
            x + g.bearing.x * txt.scale,
            y + (g.bearing.y - g.size.y) * txt.scale
        };

        gi.Size = {
            g.size.x * txt.scale,
            g.size.y * txt.scale
        };

        gi.UV_MM = { g.uvMin, g.uvMax };
        gi.Color = txt.color;

        txt.layout.glyphs.push_back(gi);

        minY = std::min(minY, gi.Offset.y);
        maxY = std::max(maxY, gi.Offset.y + gi.Size.y);

        x += (g.advance >> 6) * txt.scale;
        maxX = std::max(maxX, x);
    }

    txt.layout.width  = maxX;           //Guarda a maior largura da linha - Necessário para alinhamento, bounding box, centralização;
    txt.layout.height = maxY - minY;
    txt.dirty = false;
}


void TextRenderSystem::FLUSH(std::vector<Entity> batches, glm::mat4 mvp, bool depth) {
    auto* cam = GEngine->mainCamera;
    float W = (float)GEngine->window->GetWidth();
    float H = (float)GEngine->window->GetHeight();

    for (auto e : batches) {  
        auto& txt = scn->GetComponent<TextComponent>(e);
        auto& tr = scn->GetComponent<TransformComponent>(e); 

        Font* font = FontManager::Get(txt.fontID);
        if (!font) continue;
         
        // 1 - BUILD LAYOUT (DIRTY) 
        if (txt.dirty) BuildTextLayout(txt, font);
         
        // 2 - BASE POSITION 
        glm::vec2 basePos(0);
         
        if (txt.space == TextComponent::Space::Screen) {
            if (txt.units == TextComponent::Units::Relative)
                basePos = glm::vec2(tr.position.x * W, tr.position.y * H);
            else
                basePos = glm::vec2(tr.position);
        }
         
        // 3 - ALIGN + ANCHOR 
        float textW = txt.layout.width;
        float textH = txt.layout.height;

        // 1. Anchor (define pivot)
        //basePos = ApplyAnchor(basePos, { textW, textH }, txt.anchor);
        

         //basePos = glm::vec2(GEngine->window->GetWidth()/2, GEngine->window->GetHeight()/2);

        // 2. Align (alinha conteúdo)
        switch (txt.align) {
        case TextComponent::Align::Center:
            basePos.x -= textW * 0.5f;      break;
        case TextComponent::Align::Right:
            basePos.x -= textW;             break;
        default:
                                            break;
        }
         
        // 5 - SUBMIT CACHED GLYPHS 
        for (auto gi : txt.layout.glyphs) {
            auto& s = txt.style; 

            //Pos and Size
            gi.Offset.x += basePos.x;     
            gi.Offset.y += basePos.y;
            //Thickness x
            gi.Outline_ThicknessRGB.x = s.outlineThickness;
            //RGB       y-z-w
            gi.Outline_ThicknessRGB.y = s.outlineColor.x;
            gi.Outline_ThicknessRGB.z = s.outlineColor.y;
            gi.Outline_ThicknessRGB.w = s.outlineColor.z; 

            //Shadow Offset
            gi.ShadowA.x = txt.style.shadowOffset.x;
            gi.ShadowA.y = txt.style.shadowOffset.y;
            //ShadowColor
            gi.ShadowA.z = txt.style.shadowColor.x;
            gi.ShadowA.w = txt.style.shadowColor.y;
            gi.ShadowB.x = txt.style.shadowColor.z;
            gi.ShadowB.y = txt.style.shadowColor.w; 

            FontRenderer::Submit(font, gi);
        }
    }  
    // 6 - FLUSH (ONCE)  
    FontRenderer::Flush(mvp, false); 
}
 



void TextRenderSystem::FLUSH3D(const std::vector<Entity>& batches)
{
    auto* cam = GEngine->mainCamera;
    glm::mat4 VP = cam->GetProjectionMatrix() * cam->GetViewMatrix();

    FontRenderer::Begin();

    for (auto e : batches)
    {
        auto& txt = scn->GetComponent<TextComponent>(e);
        auto& tr = scn->GetComponent<TransformComponent>(e);

        Font* font = FontManager::Get(txt.fontID);
        if (!font) continue;

        if (txt.dirty)
            BuildTextLayout(txt, font);

        glm::vec3 anchor = tr.position; // 🔑 UM pivot só

        for (auto gi : txt.layout.glyphs)
        {
            gi.Anchor = anchor;

            auto& s = txt.style;
            gi.Outline_ThicknessRGB = {
                s.outlineThickness,
                glm::vec3(s.outlineColor.x, s.outlineColor.y, s.outlineColor.z)
            };

            gi.ShadowA = {
                s.shadowOffset,
                s.shadowColor.r,
                s.shadowColor.g
            };

            gi.ShadowB = {
                s.shadowColor.b,
                s.shadowColor.a
            };

            FontRenderer::Submit(font, gi);
        }
    }

    FontRenderer::Flush3D(
        VP,
        cam->Right,
        cam->Up,
        true
    );
} 

void FlushDebug3D(std::string text, glm::vec3 position, float scale, glm::vec3 color ) {
    Font* font = FontManager::Get(0);
    float x = 0.0f;
    float y = 0.0f;

    for (char c : text)
    {
        if (c == '\n') {
            y -= font->pixelSize * scale;
            x = 0;
            continue;
        }

        const Glyph& g = font->glyphs[c];
        GlyphInstance gi;

        gi.Anchor = position;

        gi.Offset = {
            x + g.bearing.x * scale,
            y + (g.bearing.y - g.size.y) * scale
        };

        gi.Size = {
            g.size.x * scale,
            g.size.y * scale
        };

        gi.UV_MM = { g.uvMin, g.uvMax };
        gi.Color = glm::vec4(color, 1);

        // sem outline/shadow por padrão (debug)
        gi.Outline_ThicknessRGB = { 0.05, 1,1,1 };
        gi.ShadowA = { 0,0,0,0 };
        gi.ShadowB = { 0,0 };

        FontRenderer::Submit(font, gi);

        x += (g.advance >> 6) * scale;
    }

}

 

void TextRenderSystem::DebugDrawChunkSDFGrid(
    OctreeNode* node,
    const std::vector<float>& sdf,
    Camera* cam,
    float textScale 
) {
    int N = sysv.get_voxelGrid(); // ex: 17
    glm::vec3 min = node->getBounds().min;
    glm::vec3 size = node->getBounds().max - min;

    auto idx = [&](int x, int y, int z) { return (z * N + y) * N + x; };

    FontRenderer::Begin();
    for (int z = 0; z < N; z++)   for (int y = 0; y < N; y++)   for (int x = 0; x < N; x++) {
        float v = sdf[idx(x, y, z)];

        glm::vec3 p = min + (glm::vec3(x, y, z) / float(N - 1)) * size;


        //   FILTRO CRÍTICO
        if (fabs(v) > node->voxel_size() * 3.0f)    //* 1.5f
            continue;

        // --- cor por sinal ---
        glm::vec3 color;
        if (v < 0.0f)      color = { 1, 0, 0 };   // dentro
        else if (v < 1.0f) color = { 1, 1, 0 };   // superfície
        else               color = { 0, 1, 0 };   // fora

        // ponto da grade
        DebugRenderer::Point(p, color, 4.0f, DebugPointType::Circle, false);

        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", v);

        FlushDebug3D(
            buf,
            p + glm::vec3(0, node->voxel_size() * 0.25f, 0),
            textScale,
            color 
        );

    }

    glm::mat4 VP = cam->GetProjectionMatrix() * cam->GetViewMatrix();
    FontRenderer::Flush3D(VP,
        cam->Right,
        cam->Up,
        true // billboard ON
    );  

}






void DebugDrawChunkGrid(OctreeNode* node) {
    int N = sysv.get_voxelGrid();
    glm::vec3 min = node->getBounds().min;
    glm::vec3 max = node->getBounds().max;
    glm::vec3 size = max - min;

    glm::vec3 cell = size / float(N - 1);
    glm::vec3 color(0.3f);

    for (int i = 0; i < N; i++) {
        float t = float(i) / float(N - 1);

        // fatias XY
        DebugRenderer::Cube(
            min + glm::vec3(0, 0, t * size.z),
            min + glm::vec3(size.x, size.y, t * size.z + cell.z),
            color,
            true
        );

        // fatias XZ
        DebugRenderer::Cube(
            min + glm::vec3(0, t * size.y, 0),
            min + glm::vec3(size.x, t * size.y + cell.y, size.z),
            color,
            true
        );

        // fatias YZ
        DebugRenderer::Cube(
            min + glm::vec3(t * size.x, 0, 0),
            min + glm::vec3(t * size.x + cell.x, size.y, size.z),
            color,
            true
        );
    }
}











/*



static void BuildTextLayout(TextComponent& txt, Font* font)
{
    txt.layout.glyphs.clear();

    float x = 0, y = 0;
    float maxX = 0;


    for (char c : txt.text) {
        if (c == '\n') {
            y -= font->pixelSize * txt.scale;
            x = 0;
            continue;
        }

        const Glyph& g = font->glyphs[c];   //to search in Atlas

        GlyphInstance gi;
        gi.pos = {
            x + g.bearing.x * txt.scale,
            y - (g.size.y - g.bearing.y) * txt.scale
        };
        gi.size = glm::vec2(g.size) * txt.scale;
        gi.uvMin = g.uvMin;
        gi.uvMax = g.uvMax;
        gi.color = txt.color;

        txt.layout.glyphs.push_back(gi);

        x += (g.advance >> 6) * txt.scale;
        maxX = std::max(maxX, x);
    }

    txt.layout.width = maxX;    //Guarda a maior largura da linha - Necessário para alinhamento, bounding box, centralização;
    txt.layout.height = abs(y) + font->pixelSize * txt.scale;       //txt.layout.height = font->pixelSize * txt.scale;
    txt.dirty = false;
}





 

void Update(float) override
{
    FontRenderer::Begin();

    auto* cam = GEngine->mainCamera;
    float W = (float)GEngine->window->GetWidth();
    float H = (float)GEngine->window->GetHeight();

    glm::mat4 ortho = glm::ortho(0.0f, W, 0.0f, H);

    for (auto e : scn->GetEntitiesWith<TextComponent, Transform>()) {
        auto& txt = scn->GetComponent<TextComponent>(e);
        auto& tr = scn->GetComponent<Transform>(e);

        Font* font = FontManager::Get(txt.fontID);
        if (!font) continue;

        // =========================
        // 1 - BUILD LAYOUT (DIRTY)
        // =========================
        if (txt.dirty)
            BuildTextLayout(txt, font);

        // =========================
        // 2 - BASE POSITION
        // =========================
        glm::vec2 basePos(0);

        if (txt.space == TextComponent::Space::Screen) {
            basePos = (txt.units == TextComponent::Units::Relative)
                ? glm::vec2(tr.position.x * W, tr.position.y * H)
                : glm::vec2(tr.position);
        }

        // =========================
        // 3 - ALIGN + ANCHOR
        // =========================
        float textW = txt.layout.width;
        float textH = txt.layout.height;

        if (txt.align == TextComponent::Align::Center)
            basePos.x -= textW * 0.5f;
        else if (txt.align == TextComponent::Align::Right)
            basePos.x -= textW;

        basePos = ApplyAnchor(basePos, { textW, textH }, txt.anchor);

        // =========================
        // 4 - MVP
        // =========================
        glm::mat4 mvp;
        bool depth = false;

        if (txt.space == TextComponent::Space::Screen) {
            mvp = ortho;
        }
        else {
            glm::mat4 model = glm::translate(glm::mat4(1), tr.position);

            if (txt.billboard) {
                model[0] = glm::vec4(cam->Right, 0);
                model[1] = glm::vec4(cam->Up, 0);
                model[2] = glm::vec4(cam->Front, 0);
            }

            mvp = cam->GetProjectionMatrix()
                * cam->GetViewMatrix()
                * model;

            depth = txt.depthTest;
        }

        // =========================
        // 5 - SUBMIT CACHED GLYPHS
        // =========================
        for (auto gi : txt.layout.glyphs) {
            //gi.pos += basePos;

            gi.pos += basePos;

            gi.outlineThickness = txt.style.outlineThickness;
            gi.outlineColor = txt.style.outlineColor;

            gi.shadowOffset = txt.style.shadowOffset;
            gi.shadowColor = txt.style.shadowColor;


            FontRenderer::Submit(font, gi);
        }
    }

    // =========================
    // 6 - FLUSH (ONCE)
    // =========================
    FontRenderer::Flush(ortho, false);
}

*/