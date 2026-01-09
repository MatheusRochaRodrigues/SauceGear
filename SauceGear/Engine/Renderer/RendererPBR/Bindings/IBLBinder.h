#pragma once
#include "../../../Graphics/Framebuffer.h"
#include "../IBL/IBLSet.h"

struct IBLBinder {
    static void Bind(const IBLSet& ibl) {
        glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_CUBE_MAP, ibl.irradiance);
        glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_CUBE_MAP, ibl.prefilter);
        glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D,       ibl.brdfLUT);
    }
};
