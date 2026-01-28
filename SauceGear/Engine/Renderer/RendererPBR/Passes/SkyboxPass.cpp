#include "SkyboxPass.h"
#include <array>
#include <string> 
#include <stb/stb_image.h>

// ---------- CUBEMAP PNG DIRETO ----------
static std::array<std::string, 6> faces = {
    SkyboxPass::path + "/right.png",
    SkyboxPass::path + "/left.png",
    SkyboxPass::path + "/top.png",
    SkyboxPass::path + "/bottom.png",
    SkyboxPass::path + "/front.png",
    SkyboxPass::path + "/back.png"
}; 


GLuint SkyboxPass::LoadCubemap2TEX()    //const std::array<std::string, 6>& faces
{
    stbi_set_flip_vertically_on_load(false);

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    int w, h, comp;
    for (int i = 0; i < 6; i++) {
        float* data = stbi_loadf(faces[i].c_str(), &w, &h, &comp, 3);
        if (!data) {
            std::cerr << "[IBL] Falha ao carregar HDR face: " << faces[i] << "\n";
            continue;
        }

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            GL_RGBA,       //GL_RGB16F
            w, h,
            0,
            GL_RGB,
            GL_FLOAT,
            data
        );

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return texID;
}
