#pragma once
#include "../ECS/System.h"
#include "../Scene/SceneECS.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Renderer.h"

#include "../DebugRender/DebugLinesRenderer.h"
#include "../DebugRender/DebugWireframeRenderer.h"
#include "../DebugRender/DebugPointRenderer.h"

#include "../Geometry/WorldOctree/SurfaceNets/OctreeNode.h"

class DebugRenderer : public System {
public:
    DebugRenderer() {
        lineShader = new Shader("../../DebugRender/Shader/debug_line.vs", "../../DebugRender/Shader/debug_line.fs");
        wireShader = new Shader("../../DebugRender/Shader/debug_wire.vs", "../../DebugRender/Shader/debug_wire.fs");
        pointShader = new Shader("../../DebugRender/Shader/debug_point.vs", "../../DebugRender/Shader/debug_point.fs");

        lineRenderer.Init(lineShader);
        DebugWireframeRenderer::Init(wireShader);
        pointRenderer.Init(pointShader);
    }

    static void Line(const glm::vec3& a,
        const glm::vec3& b,
        const glm::vec3& color = glm::vec3(1,0,0),
        bool persistent = false)
    {
        lineRenderer.AddLine(a, b, color, persistent);
    }

    static void Point(
        const glm::vec3& pos,
        const glm::vec3& color = glm::vec3(1, 1, 1),
        float size = 6.0f,
        DebugPointType type = DebugPointType::Square,
        bool persistent = false)
    {
        pointRenderer.AddPoint(pos, color, size, type, persistent);
    }
     
    static void Cube(
        const glm::vec3& min,
        const glm::vec3& max,
        const glm::vec3& color = glm::vec3(1, 1, 1),
        bool persistent = false)
    {
        glm::vec3 v[8] = {
            {min.x, min.y, min.z},
            {max.x, min.y, min.z},
            {max.x, max.y, min.z},
            {min.x, max.y, min.z},

            {min.x, min.y, max.z},
            {max.x, min.y, max.z},
            {max.x, max.y, max.z},
            {min.x, max.y, max.z}
        };

        // base
        Line(v[0], v[1], color, persistent);
        Line(v[1], v[2], color, persistent);
        Line(v[2], v[3], color, persistent);
        Line(v[3], v[0], color, persistent);

        // topo
        Line(v[4], v[5], color, persistent);
        Line(v[5], v[6], color, persistent);
        Line(v[6], v[7], color, persistent);
        Line(v[7], v[4], color, persistent);

        // colunas
        Line(v[0], v[4], color, persistent);
        Line(v[1], v[5], color, persistent);
        Line(v[2], v[6], color, persistent);
        Line(v[3], v[7], color, persistent);
    }

    void Update(float dt) override {
        try {
            GEngine->renderer->frameScreen->Bind();

            auto entities = GEngine->scene->GetEntitiesWith<DebugMeshComponent>();

            for (auto e : entities) {
                //estou obrigando nesse instante a qeum tem debug mesh component possuir esses components abaixo
                auto& dbg = GEngine->scene->GetComponent<DebugMeshComponent>(e);
                auto& mr = GEngine->scene->GetComponent<MeshRenderer>(e);
                auto& tr = GEngine->scene->GetComponent<TransformComponent>(e);

                if (dbg.showWireframe)
                    DebugWireframeRenderer::Draw(mr.mesh, tr.GetMatrix(), dbg.color);

                if (dbg.showBox) { 
                    if (e != INVALID_ENTITY) {
                        if (!GEngine->scene->HasComponent<AABBComponent>(e)) continue;
                        auto aabb = GEngine->scene->GetComponent<AABBComponent>(e);
                        DebugRenderer::Cube( aabb.worldMin, aabb.worldMax, dbg.colorBox, false );
                    }
                }

                if (!GEngine->scene->HasComponent<SurfaceNetsComponent>(e)) continue;
                auto& sf = GEngine->scene->GetComponent<SurfaceNetsComponent>(e);
                if(sf.showBoxOctree) {
                    auto aabb = sf.node->getBounds(); 
                    DebugRenderer::Cube(aabb.min, aabb.max, dbg.colorBox, false);
                    DebugDrawChunkGrid(sf.node);
                }
                
            }

            lineRenderer.Update(dt);
            pointRenderer.Update(dt);


            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        } catch (const std::exception& e) {
            std::cerr << "[EXCEÇÃO - DebugRenderer] " << e.what() << "\n";
        }
    }



    void DebugDrawChunkGrid(OctreeNode* node) {
        int N = sysv.get_voxelGrid(); // ex: 17
        glm::vec3 min = node->getBounds().min;
        glm::vec3 max = node->getBounds().max;
        glm::vec3 size = max - min;

        glm::vec3 cell = size / float(N - 1);

        for (int z = 0; z < N - 1; z++)
            for (int y = 0; y < N - 1; y++)
                for (int x = 0; x < N - 1; x++) {

                    glm::vec3 cmin = min + glm::vec3(x, y, z) * cell;
                    glm::vec3 cmax = cmin + cell;

                    DebugRenderer::Cube(
                        cmin,
                        cmax,
                        glm::vec3(0.5f,0,0),
                        false
                    );
                }
    }




    static inline glm::vec3 ColorByDepth(int depth) {
        static const glm::vec3 palette[] = {
            {1, 0, 0}, // depth 0 - vermelho
            {0, 1, 0}, // depth 1 - verde
            {0, 0, 1}, // depth 2 - azul
            {1, 1, 0}, // depth 3 - amarelo
            {1, 0, 1}, // depth 4 - magenta
            {0, 1, 1}, // depth 5 - ciano
        };

        constexpr int count = sizeof(palette) / sizeof(palette[0]);
        return palette[depth % count];
    }

private:
    static inline DebugLineRenderer lineRenderer;
    static inline DebugPointRenderer pointRenderer;

    Shader* lineShader = nullptr;
    Shader* wireShader = nullptr;
    Shader* pointShader = nullptr;
};
