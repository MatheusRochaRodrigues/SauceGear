#pragma once
#include "../../../Graphics/Framebuffer.h"

struct GBufferBinder {
    static void Bind(const Framebuffer& g) {
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, g.GetTextureByType(FramebufferTextureType::Position));
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, g.GetTextureByType(FramebufferTextureType::Normal));
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, g.GetTextureByType(FramebufferTextureType::Albedo));
        glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, g.GetTextureByType(FramebufferTextureType::MetallicRoughnessAO));
    }
};
 