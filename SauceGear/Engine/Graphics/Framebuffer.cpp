#include "Framebuffer.h"
#include <iostream>

Framebuffer::Framebuffer(int width, int height, const std::vector<FramebufferAttachment>& attachments)
    : width(width), height(height), attachmentSpecs(attachments) {
    notHasDepth = true;
    Init();
}

Framebuffer::Framebuffer(int width, int height, const std::vector<FramebufferAttachment>& attachments, bool useDepthRB)
    : width(width), height(height), attachmentSpecs(attachments), useRenderbuffer(useDepthRB) {
    Init();
}

Framebuffer::Framebuffer(int width, int height, const std::vector<FramebufferAttachment>& attachments, GLuint sdRBO, bool useFallbackRB)
    : width(width), height(height), attachmentSpecs(attachments), sharedRBO(sdRBO), useRenderbuffer(useFallbackRB) {
    Init();
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &fbo);
    for (auto tex : colorTextures)
        glDeleteTextures(1, &tex);
    if (depthTexture) glDeleteTextures(1, &depthTexture);
    if (depthRBO && ownsDepthRBO) glDeleteRenderbuffers(1, &depthRBO);
}

void Framebuffer::Init() {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    colorTextures.resize(attachmentSpecs.size());
    glGenTextures((GLsizei)colorTextures.size(), colorTextures.data());

    GLenum drawBuffers[16]; // suporte até 16 attachments
    int drawIndex = 0;

    for (size_t i = 0; i < attachmentSpecs.size(); ++i) {
        auto type = attachmentSpecs[i].type;
        GLenum format = GL_RGB, internalFormat = GL_RGB, dataType = GL_UNSIGNED_BYTE;

        switch (type) {
        case FramebufferTextureType::Color:
            internalFormat = GL_RGBA8;
            format = GL_RGBA;
            break;
        case FramebufferTextureType::Float16:
            internalFormat = GL_RGBA16F;
            format = GL_RGBA;
            dataType = GL_FLOAT;
            break;
        case FramebufferTextureType::Integer:
            internalFormat = GL_R32I;
            format = GL_RED_INTEGER;
            dataType = GL_INT;
            break;
        case FramebufferTextureType::ColorRGB:
            internalFormat = GL_RGB;
            format = GL_RGB;
            break;
        case FramebufferTextureType::ColorRGBA:
            internalFormat = GL_RGBA;
            format = GL_RGBA;
            break;

        //Deffered
        case FramebufferTextureType::Position:
            internalFormat = GL_RGB16F;         //RGBA  -opcional
            format = GL_RGB;                    //RGBA  -opcional
            dataType = GL_FLOAT;
            break;
        case FramebufferTextureType::Normal:
            internalFormat = GL_RGB16F;
            format = GL_RGB;
            dataType = GL_FLOAT;
            break;
        case FramebufferTextureType::Albedo:
            internalFormat = GL_RGBA8;      //GL_RGBA8
            format = GL_RGBA;
            break;

        //Only Channels
        case FramebufferTextureType::REDFloat:
            internalFormat = GL_R16F;       //GL_RED
            format = GL_RED;
            dataType = GL_FLOAT;
            break; 

        //PBR
        case FramebufferTextureType::MetallicRoughnessAO:
            internalFormat = GL_RGBA8;      //GL_RGBA8          R = Metallic, G = Roughness, B = AO, A = ?
            format = GL_RGBA;
            break;
             
        case FramebufferTextureType::HDR:
            internalFormat = GL_RGBA16F;
            format = GL_RGBA;
            dataType = GL_FLOAT;
            break;
    }


        glBindTexture(GL_TEXTURE_2D, colorTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataType, nullptr);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        //New
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorTextures[i], 0);
        drawBuffers[drawIndex++] = GL_COLOR_ATTACHMENT0 + i;
    }

    if (drawIndex > 0)
        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        glDrawBuffers(drawIndex, drawBuffers);
    else
        glDrawBuffer(GL_NONE);

    if (!notHasDepth) {
        // create and attach depth buffer (renderbuffer)
        if (sharedRBO) {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                GL_DEPTH_STENCIL_ATTACHMENT,
                GL_RENDERBUFFER,
                sharedRBO
            );
            depthRBO = sharedRBO;
            ownsDepthRBO = false;

        }
        else if (useRenderbuffer) {
            glGenRenderbuffers(1, &depthRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRBO);

            ownsDepthRBO = true;
        }
        else {
            glGenTextures(1, &depthTexture);
            glBindTexture(GL_TEXTURE_2D, depthTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
        }
    }

    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[Framebuffer] Incompleto!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// optional you can use just only GL_DEPTH_ATTACHMENT
//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

void Framebuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::Unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint Framebuffer::GetTexture(int index) const {
    return colorTextures.at(index);
}

GLuint Framebuffer::GetDepthTexture() const {
    return depthTexture;
}

void Framebuffer::Resize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;

    // Recria tudo
    glDeleteFramebuffers(1, &fbo);
    for (auto tex : colorTextures) glDeleteTextures(1, &tex);
    if (depthTexture) glDeleteTextures(1, &depthTexture);
    if (depthRBO && ownsDepthRBO) glDeleteRenderbuffers(1, &depthRBO);

    fbo = 0; depthTexture = 0; depthRBO = 0;
    Init();
}



//           Exemplo de Uso
//std::vector<FramebufferAttachment> attachments = {
//    {FramebufferTextureType::Float16, "SceneColor"},
//    {FramebufferTextureType::Float16, "Bloom"}
//};
//
//Framebuffer postProcessFBO(1280, 720, attachments);


void Framebuffer::CreateFramebuffer(GLuint& fbo, GLuint& texture) {
    unsigned int SCR_WIDTH = GEngine->window->m_width;
    unsigned int SCR_HEIGHT = GEngine->window->m_height;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}







//void verify() {
//    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//    if (status != GL_FRAMEBUFFER_COMPLETE)
//        std::cout << "Framebuffer incompleto: " << status << std::endl;
//}