#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>

#include "../Core/EngineContext.h"
#include "../Platform/Window.h"
#include "SharedDepthStencil.h"

enum class FramebufferTextureType {
    Color, Depth, Float16, Integer, ColorRGB, ColorRGBA,
    //Only Channels
    REDFloat, //R16F, 
    //Deffered
    Position, Normal, Albedo,    
    //PBR
    MetallicRoughnessAO,
    HDR,

     
    ColorRGBA8,
    RGBA16F,
    RGB16F,
    R16F,
    R32I 
};

struct FramebufferAttachment {
    FramebufferTextureType type;
    std::string name; // opcional, útil para debug
};

class Framebuffer {
public:
    // Sem profundidade
    Framebuffer(int width, int height, const std::vector<FramebufferAttachment>& attachments);
    // RBO próprio
    Framebuffer(int width, int height, const std::vector<FramebufferAttachment>& attachments, bool useDepthRenderbuffer);
    // RBO compartilhado
    Framebuffer(int width, int height, const std::vector<FramebufferAttachment>& attachments, GLuint sdRBO, bool useFallbackRB);
    
    ~Framebuffer();

    void Bind() const;
    void Unbind() const;
    void Resize(int newWidth, int newHeight);

    GLuint GetTexture(int index) const;
    GLuint GetDepthTexture() const;
    GLuint GetID() const { return fbo; }
    GLuint GetRBO() const { return depthRBO; }

    int GetWidth()  { return width;  }
    int GetHeight() { return height; }

    void Clear() const;

    GLuint GetTextureByType(FramebufferTextureType type) const {
        int i = 0;
        for (auto& attachment : attachmentSpecs) {
            if (attachment.type == type) return colorTextures[i];
            i++;
        }
        return 0;

        /*auto it = attachmentSpecs.find(type);
        if (it != attachmentSpecs.end())
            return it->second;
        return 0;*/
    }

    //global
    static void CreateFramebuffer(GLuint& fbo, GLuint& texture);
private:
    void Init();

    int width, height;
    GLuint fbo = 0;
    std::vector<FramebufferAttachment> attachmentSpecs;
    //std::unordered_map<FramebufferTextureType, GLuint> textureMap;
    std::vector<GLuint> colorTextures;
    GLuint depthTexture = 0;
    GLuint depthRBO = 0;
    bool useRenderbuffer = false;
    bool notHasDepth = false;
     
    bool ownsDepthRBO = false;   
    GLuint sharedRBO = 0;

};

//to future, Like A Vulkan struct to Data Info
/*
struct RenderTarget {
    int width, height;
    SharedDepthStencil depth;
    Framebuffer gbuffer;
    Framebuffer lighting;
    Framebuffer post;
};
*/