#pragma once 
#include <vector>
#include <string>
#include <glm/glm.hpp> 
#include "../ECS/System.h"
#include "../Scene/SceneECS.h"
#include "../Core/EngineContext.h"
#include "../Platform/Window.h"
#include "../Graphics/Shader.h"  
#include "../Core/Camera.h"
#include "../Components/TextComponent.h" 
#include "../Components/Transform.h" 
#include "../GUI/FontRenderer.h" 
#include "../GUI/FontManager.h" 

class TextRenderSystem : public System{
public:
    static void Init();
    static void Shutdown();

    TextRenderSystem() {
        //FontRenderer::SetShader(new Shader("GUI/AtlasTextRender.vs", "GUI/AtlasTextRender.fs"));
        FontRenderer::SetShader(new Shader("GUI/AtlasTextMSDF.vs", "GUI/AtlasTextMSDF.fs"));
        FontManager::Load("Assets/Fonts/Roboto-Medium.ttf", 48);
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

      
private:
    static void BuildTextLayout(
        TextComponent& txt,
        Font* font)
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

            const Glyph& g = font->glyphs[c];

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

        txt.layout.width = maxX;
        txt.layout.height = font->pixelSize * txt.scale;
        txt.dirty = false;
    }

    static glm::vec2 ApplyAnchor(
        const glm::vec2& pos,
        const glm::vec2& size,
        TextComponent::Anchor anchor)
    {
        glm::vec2 p = pos;

        switch (anchor) {
        case TextComponent::Anchor::TopCenter:     p.x -= size.x * 0.5f; break;
        case TextComponent::Anchor::TopRight:      p.x -= size.x; break;

        case TextComponent::Anchor::CenterLeft:    p.y -= size.y * 0.5f; break;
        case TextComponent::Anchor::Center:        p -= size * 0.5f; break;
        case TextComponent::Anchor::CenterRight:   p.x -= size.x; p.y -= size.y * 0.5f; break;

        case TextComponent::Anchor::BottomLeft:    p.y -= size.y; break;
        case TextComponent::Anchor::BottomCenter:  p.x -= size.x * 0.5f; p.y -= size.y; break;
        case TextComponent::Anchor::BottomRight:   p -= size; break;

        default: break; // TopLeft
        }
        return p;
    }

    static float MeasureTextWidth(Font* font, const std::string& text, float scale)
    {
        float w = 0.0f;
        for (char c : text)
            w += (font->glyphs[c].advance >> 6) * scale;
        return w;
    }

};









/*
    static inline std::vector<TextComponent> texts; 

 void RenderTextWorld(
        const std::string& text,
        const glm::vec3& pos,
        float scale,
        const glm::vec3& color,
        Camera* cam)
    {
        glm::mat4 model = glm::translate(glm::mat4(1), pos);

        // billboard
        model[0] = glm::vec4(cam->Right, 0);
        model[1] = glm::vec4(cam->Up, 0);
        model[2] = glm::vec4(cam->Front, 0);        //Forward

        glm::mat4 mvp = cam->GetProjectionMatrix() * cam->GetViewMatrix() * model;
        FontRenderer::Draw(text, mvp, scale, color);
    }

    void RenderTextScreen(
        const std::string& text,
        const glm::vec2& pos,
        float scale,
        const glm::vec3& color)
    {
        glm::mat4 ortho = glm::ortho( 0.0f, (float)GEngine->window->GetWidth(), 0.0f, (float)GEngine->window->GetHeight() );
        FontRenderer::Draw(text, ortho, pos, scale, color);
    }





static void Text3D(          //TextWorld
        const glm::vec3& pos,
        const std::string& text,
        const glm::vec3& color = { 1,1,1 },
        float scale = 0.05f,
        bool persistent = false
    ) {
        texts.push_back({ text, pos, color, scale, true, persistent });
    };

    static void Text2D(          //TextScreen
        const glm::vec2& pos,
        const std::string& text,
        const glm::vec3& color = { 1,1,1 },
        float scale = 1.0f,
        bool persistent = false
    ) {
        texts.push_back({ text, glm::vec3(pos, 0), color, scale, false, persistent });
    };





    void Update(float dt) override {
        FontRenderer::Begin();

        for (auto e : scn->GetEntitiesWith<TextComponent, Transform>()) {
            auto& text = scn->GetComponent<TextComponent>(e);
            auto& tr =      scn->GetComponent<Transform>(e);

            if (!text.dirty) continue;

            Font* font = FontManager::Get(text.fontID);

            float x = 0;
            for (char c : text.text) {
                const Glyph& g = font->glyphs[c];

                GlyphInstance inst;
                inst.pos    = { x + g.bearing.x, g.bearing.y };
                inst.size   = g.size;
                inst.uvMin  = g.uvMin;
                inst.uvMax  = g.uvMax;
                inst.color  = text.color;

                FontRenderer::Submit(font, inst);
                x += (g.advance >> 6) * text.scale;
            }
            text.dirty = false;

        }

        FontRenderer::Flush(cameraMVP);

    };
*/