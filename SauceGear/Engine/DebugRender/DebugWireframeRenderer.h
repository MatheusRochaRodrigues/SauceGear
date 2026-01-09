#pragma once
#include "../ECS/System.h"
#include "../Core/EngineContext.h"
#include "../Core/Camera.h"
#include "../Graphics/Shader.h"
#include "../Graphics/PrimitiveMesh.h"
#include "WireframeCache.h"
#include <glm/glm.hpp>

class DebugWireframeRenderer {
public:
    static void Init(Shader* shader) {
        wfShader = shader;
    }

    static void Draw(MeshInstance* mesh,    //Mesh
        const glm::mat4& model,
        const glm::vec3& color)
    {
        if (!mesh || !wfShader) return;

        auto* wf = WireframeCache::Get(mesh);

        glm::mat4 vp =
            GEngine->mainCamera->GetProjectionMatrix() *
            GEngine->mainCamera->GetViewMatrix();

        wfShader->use();
        wfShader->setMat4("uVP", vp);
        wfShader->setMat4("uModel", model);
        wfShader->setVec3("uColor", color);


        glLineWidth(2.0f);
        glEnable(GL_POLYGON_OFFSET_LINE);
        //glPolygonOffset(-1.0f, -1.0f);
        glPolygonOffset(-2.0f, -2.0f);   // se ainda piscar

        glBindVertexArray(wf->vao);
        glDrawArrays(GL_LINES, 0, wf->vertexCount);
        glBindVertexArray(0);

        glDisable(GL_POLYGON_OFFSET_LINE);
    }

private:
    static inline Shader* wfShader = nullptr;
};
