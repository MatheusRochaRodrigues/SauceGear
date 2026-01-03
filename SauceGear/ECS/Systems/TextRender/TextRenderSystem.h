#pragma once 
#include <vector>
#include <string>
#include <glm/glm.hpp> 
#include "../../ECS/System.h"
#include "../../Scene/SceneECS.h"
#include "../../Core/EngineContext.h"
#include "../../Graphics/Renderer.h"
#include "../../Platform/Window.h"
#include "../../Graphics/Shader.h"  
#include "../../Core/Camera.h"
#include "../../Components/TextComponent.h" 
#include "../../Components/TransformComponent.h" 
#include "../../GUI/FontRenderer.h" 
#include "../../GUI/FontManager.h"

#include "../../GUI/ToLetter3D.h" 
#include "../../Geometry/WorldOctree/SurfaceNets/SysVoxel.h" 
#include "../../Geometry/WorldOctree/SurfaceNets/OctreeNode.h" 
#include "../DebugRenderer.h"
#include "../../Geometry/World/SurfaceNets/SurfaceNets.h" 
 


class TextRenderSystem : public System{
public:
    static void Init();         static void Shutdown();

    TextRenderSystem() { 
        FontRenderer::SetShader(new Shader("GUI/AtlasTextSDF.vs", "GUI/AtlasTextSDF.fs"));
        FontManager::Load("Assets/Fonts/Roboto-Medium.ttf", 48);

        FontRenderer::Init();
    }  

    void Update(float dt) override {
        try {
            GEngine->renderer->frameScreen->Bind();
             
            float W = (float)GEngine->window->GetWidth();
            float H = (float)GEngine->window->GetHeight(); 

            std::vector<Entity> screenTexts;
            std::vector<Entity> worldTexts;

            //2D
            for (auto e : scn->GetEntitiesWith<TextComponent, TransformComponent>()) {
                auto& txt = scn->GetComponent<TextComponent>(e);
                if (txt.space == TextComponent::Space::Screen) screenTexts.push_back(e);
                else worldTexts.push_back(e);
            }
            FontRenderer::Begin();
            //glm::mat4 ortho = glm::ortho(0.0f, W, 0.0f, H);
            //glm::mat4 ortho = glm::ortho( 0.0f, W, H, 0.0f, -1.0f, 1.0f );
            glm::mat4 ortho = glm::ortho(0.0f, W, 0.0f, H, -1.0f, 1.0f);

            FLUSH(screenTexts, ortho, false);

            //3D
            FLUSH3D(worldTexts);
              
            auto entities = GEngine->scene->GetEntitiesWith<SurfaceNetsComponent>();
            for (auto e : entities) {
                auto& sf = GEngine->scene->GetComponent<SurfaceNetsComponent>(e);
                if (sf.showBoxOctree)
                    DebugDrawChunkSDFGrid(sf.node, sf.chunk->buff->densityMap, GEngine->mainCamera, 0.004f);
            }


            glBindFramebuffer(GL_FRAMEBUFFER, 0);  
        } catch (const std::exception& e) {
            std::cerr << "[EXCEÇÃO - RenderSystem] " << e.what() << "\n";
        } 
    }


private:
    void DebugDrawChunkSDFGrid(
        OctreeNode* node,
        const std::vector<float>& sdf,
        Camera* cam,
        float textScale = 0.01f
    );

    void FLUSH3D(const std::vector<Entity>& batches);
    void FLUSH(std::vector<Entity> batches, glm::mat4 mvp, bool depth); 

    static glm::vec2 ApplyAnchor( const glm::vec2& pos, const glm::vec2& size, TextComponent::Anchor anchor) {
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

    static float MeasureTextWidth(Font* font, const std::string& text, float scale) {
        float w = 0.0f;
        for (char c : text)
            w += (font->glyphs[c].advance >> 6) * scale;
        return w;
    }

};








/*

 
            if (t3d == nullptr)t3d = new TextRenderer3D();

            auto entities = GEngine->scene->GetEntitiesWith<SurfaceNetsComponent>();
            for (auto e : entities) {
                auto& sf = GEngine->scene->GetComponent<SurfaceNetsComponent>(e);
                if (sf.showBoxOctree)
                    DebugDrawChunkSDFGrid(sf.node, sf.chunk->buff->densityMap, GEngine->mainCamera, 0.003f);
            }

            t3d->RenderText3D(
                GEngine->mainCamera,
                "DEBUG TEXT 3D",
                glm::vec3(0, 2, 0),
                0.01f,
                glm::vec3(1, 1, 0),
                true
            );

            return;
             


*/




/*

    ToLetter* t = nullptr;
    void Update(float dt) override {
        try {
            GEngine->renderer->frameScreen->Bind();

            float W = (float)GEngine->window->GetWidth();
            float H = (float)GEngine->window->GetHeight();

            if(t == nullptr) t = new ToLetter(W, H);

            t->Update();

            return;
*/






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