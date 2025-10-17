#pragma once
#include "../../Core/EngineContext.h"
#include "../../ECS/Components/ComponentsHelper.h"
#include <vector>

#include "../Graphics/FullscreenQuad.h"

#define CASCADE_COUNT 5 //4

class CascadeSun {
public:
    static inline GLuint cascadeDepthMapArray = 0;
    static inline GLuint cascadeFBO = 0;
    static inline GLuint cascadeMatricesUBO = 0;
    static inline bool initialized = false;
    static inline std::vector<glm::mat4> lightSpaceMatrices;
    static inline std::vector<float> cascadePlaneDistances;

    static void Init();
    // Atualiza cascaded shadow maps do sol a partir do LightComponent (assume directional)
    static void UpdateSunShadow(LightComponent& sun, Transform& transform);

    static void GetLightSpaceMatrices(Transform& transform);   //std::vector<glm::mat4>

    static int GetCascadeCount() { return (int)cascadePlaneDistances.size(); }

    static const std::vector<float>& GetCascadeDistances() { return cascadePlaneDistances; }
};








//GLuint debugLayerTex;
//glGenTextures(1, &debugLayerTex);
//glBindTexture(GL_TEXTURE_2D, debugLayerTex);
//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
//    2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
//
//int layer = 2; // camada que você quer visualizar
//glCopyImageSubData(CascadeSun::cascadeDepthMapArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer,
//    debugLayerTex, GL_TEXTURE_2D, 0, 0, 0, 0,
//    2048, 2048, 1);
 

class DebugDepthUtils {
public:
    // desenha uma layer do shadow map em uma textura 2D pronta para debug
    static GLuint CreateDebugLayerTexture(int layer) {
        if (layer < 0 || layer >= CascadeSun::GetCascadeCount()) return 0;

        // 1️⃣ cria a textura 2D
        GLuint debugLayerTex;
        glGenTextures(1, &debugLayerTex);
        glBindTexture(GL_TEXTURE_2D, debugLayerTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 2048, 2048, 0, GL_RED, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // 2️⃣ copia a layer do array
        glCopyImageSubData(
            CascadeSun::cascadeDepthMapArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer,
            debugLayerTex, GL_TEXTURE_2D, 0, 0, 0, 0,
            2048, 2048, 1
        );

        return debugLayerTex;
    }

    // 3️⃣ opcional: shader de debug para mostrar depth
    static Shader* GetDebugShader() {
        static Shader* debugShader = nullptr;
        if (!debugShader) {
            debugShader = new Shader(
                "debugQuad.vs",
                "debugQuadDepth.fs"
            );
        }
        return debugShader;
    }

    // 4️⃣ desenha a textura no quad
    static void DrawDebugLayer(GLuint tex2D) {
        if (!tex2D) return;

        Shader* shader = GetDebugShader();
        shader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex2D);
        shader->setInt("depthTex", 0);

        // desenha quad full screen (supondo que você tenha quadMesh)
        RenderQuad();
    }

    // 4️⃣ desenha a textura no quad
    static void DrawDebugShadowLayerTex2D() {
        Shader* shader = GetDebugShader();
        shader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, CascadeSun::cascadeDepthMapArray);
        shader->setInt("depthMap", 0);
        shader->setInt("layer", 2);

        shader->setFloat("near_plane", GEngine->mainCamera->nearClip);
        shader->setFloat("far_plane", GEngine->mainCamera->farClip);

        // desenha quad full screen (supondo que você tenha quadMesh)
        RenderQuad();
    }
};

//use
//int layer = 2; // layer que você quer ver
//GLuint debugLayerTex = DebugDepthUtils::CreateDebugLayerTexture(layer);
//// ou desenhar direto no quad de debug
//DebugDepthUtils::DrawDebugLayer(debugLayerTex);
//
//// já pronto para passar pro renderer
//GEngine->renderer->GetTextureRendered = debugLayerTex;