#pragma once
#include "../../ECS/Systems/DebugRenderer.h"
#include "../../Core/EngineContext.h"
//#include "../../ECS/SceneECS.h"
//#include "../Graphics/Renderer/PBRRenderer.h"

#include "../../ECS/Components/OutlineComponent.h"
#include "../../ECS/Components/MeshRenderer.h"
#include "../../ECS/Components/Transform.h"

class OutlineSystem : public System {
public:
    void Update(float dt) override {
        auto* scene = GEngine->scene;
        Entity selected = scene->GetSelectedEntity();
        if (selected == INVALID_ENTITY) return;

        if (!scene->HasComponent<OutlineComponent>(selected) ||
            !scene->GetComponent<OutlineComponent>(selected).enabled)
            return;

        RenderOutline(selected);
    }

private:
    void RenderOutline(Entity e) {
    //    auto& tr = GEngine->scene->GetComponent<Transform>(e);
    //    auto& mr = GEngine->scene->GetComponent<MeshRenderer>(e);
    //    auto& sel = GEngine->scene->GetComponent<OutlineComponent>(e);

    //    // ====== STENCIL OUTLINE ======
    //    glEnable(GL_STENCIL_TEST);
    //    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    //    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    //    glStencilMask(0xFF);
    //    glClear(GL_STENCIL_BUFFER_BIT);

    //    // 1) Render normal meshes no stencil buffer
    //    for (auto& [material, meshes] : mr.batches) {
    //        material->Apply(&GEngine->renderer->shaders.gbuffer);
    //        for (auto& sub : meshes)
    //            mr.DrawSubM(sub);
    //    }

    //    // 2) Render enlarged mesh apenas onde stencil != 1
    //    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //    glStencilMask(0x00); // não escreve mais no stencil
    //    glDisable(GL_DEPTH_TEST);

    //    outlineShader.use();
    //    outlineShader.setMat4("model", tr.GetMatrix());
    //    outlineShader.setMat4("view", GEngine->mainCamera->GetViewMatrix());
    //    outlineShader.setMat4("projection", GEngine->mainCamera->GetProjectionMatrix());
    //    outlineShader.setVec3("outlineColor", sel.outlineColor);
    //    outlineShader.setFloat("outlineWidth", sel.outlineWidth);

    //    for (auto& [material, meshes] : mr.batches) {
    //        for (auto& sub : meshes)
    //            mr.DrawSubM(sub);
    //    }

    //    glEnable(GL_DEPTH_TEST);
    //    glStencilMask(0xFF);
    //    glDisable(GL_STENCIL_TEST);
    //}

    //// Shader simples para desenhar outline
    //Shader outlineShader = Shader("Shaders/Outline.vs", "Shaders/Outline.fs");
};














/*
class OutlineSystem : public System {
public:
    void Update(float dt) override {
        auto* scene = GEngine->scene;
        Entity selected = scene->GetSelectedEntity();
        if (selected == INVALID_ENTITY) return;

        if (!scene->HasComponent<OutlineComponent>(selected) ||
            !scene->GetComponent<OutlineComponent>(selected).enabled)
            return;

        RenderOutline(selected);
    }

private:
    void RenderOutline(Entity e) {
        auto& tr = GEngine->scene->GetComponent<Transform>(e);
        auto& mr = GEngine->scene->GetComponent<MeshRenderer>(e);
        auto& sel = GEngine->scene->GetComponent<OutlineComponent>(e);

        // ====== STENCIL OUTLINE ======
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);

        // 1) Render normal meshes no stencil buffer
        for (auto& [material, meshes] : mr.batches) {
            material->Apply(&GEngine->renderer->shaders.gbuffer);
            for (auto& sub : meshes)
                mr.DrawSubM(sub);
        }

        // 2) Render enlarged mesh apenas onde stencil != 1
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00); // não escreve mais no stencil
        glDisable(GL_DEPTH_TEST);

        outlineShader.use();
        outlineShader.setMat4("model", tr.GetMatrix());
        outlineShader.setMat4("view", GEngine->mainCamera->GetViewMatrix());
        outlineShader.setMat4("projection", GEngine->mainCamera->GetProjectionMatrix());
        outlineShader.setVec3("outlineColor", sel.outlineColor);
        outlineShader.setFloat("outlineWidth", sel.outlineWidth);

        for (auto& [material, meshes] : mr.batches) {
            for (auto& sub : meshes)
                mr.DrawSubM(sub);
        }

        glEnable(GL_DEPTH_TEST);
        glStencilMask(0xFF);
        glDisable(GL_STENCIL_TEST);
    }

    // Shader simples para desenhar outline
    Shader outlineShader = Shader("Shaders/Outline.vs", "Shaders/Outline.fs");
};
*/