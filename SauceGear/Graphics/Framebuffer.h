#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>

#include "../Core/EngineContext.h"
#include "../Platform/Window.h"

enum class FramebufferTextureType {
    Color, Depth, Float16, Integer, ColorRGB, ColorRGBA,
    //Deffered
    Position, Normal, Albedo    
};

struct FramebufferAttachment {
    FramebufferTextureType type;
    std::string name; // opcional, útil para debug
};

class Framebuffer {
public:
    Framebuffer(int width, int height, const std::vector<FramebufferAttachment>& attachments, bool useDepthRenderbuffer = false);
    ~Framebuffer();

    void Bind() const;
    void Unbind() const;
    void Resize(int newWidth, int newHeight);

    GLuint GetTexture(int index) const;
    GLuint GetDepthTexture() const;
    GLuint GetID() const { return fbo; }

    int GetWidth()  { return width;  }
    int GetHeight() { return height; }

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


};
