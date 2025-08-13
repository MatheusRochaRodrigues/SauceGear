#ifndef IBL_MAP_GENERATOR_H
#define IBL_MAP_GENERATOR_H

#include <stb/stb_image.h>
//#include <stb/stb_image_write.h>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

#include "Shader.h"
#include "RenderUtils.h" // renderCube, renderQuad

class IBLMapGenerator {
public:
    IBLMapGenerator(const Shader& equirectShader, const Shader& irradianceShader,
        const Shader& prefilterShader, const Shader& brdfShader);

    void Generate(const std::string& hdrPath);

    GLuint GetEnvironmentMap() const { return envCubemap; }
    GLuint GetIrradianceMap() const { return irradianceMap; }
    GLuint GetPrefilterMap() const { return prefilterMap; }
    GLuint GetBrdfLUT() const { return brdfLUTTexture; }

private:
    Shader equirectShader;
    Shader irradianceShader;
    Shader prefilterShader;
    Shader brdfShader;

    GLuint captureFBO, captureRBO;
    GLuint envCubemap, irradianceMap, prefilterMap, brdfLUTTexture;
    GLuint hdrTexture;

    glm::mat4 captureProjection;
    glm::mat4 captureViews[6];

    void SetupCapture();
    void LoadHDR(const std::string& hdrPath);
    void CreateCubemap();
    void CreateIrradianceMap();
    void CreatePrefilterMap();
    void CreateBRDFLUT();

    /*bool LoadFromDisk(const std::string& hdrPath);
    void SaveToDisk(const std::string& hdrPath);*/
};

#endif // IBL_MAP_GENERATOR_H