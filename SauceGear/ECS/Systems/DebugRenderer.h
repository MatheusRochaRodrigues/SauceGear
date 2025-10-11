#pragma once
#include "../Resources/DebugRender/DebugLinesRenderer.h"
#include "../Resources/DebugRender/DebugPointRenderer.h" 
#include "../Resources/DebugRender/DebugPointDrawer.h" 


#include "../Core/EngineContext.h" 
#include "../Graphics/Renderer.h" 
#include "../Scene/SceneECS.h" 

class DebugRenderer : public System {
public:
    DebugRenderer() {
        Init(
            new Shader("DebugP/debug_line.vs", "DebugP/debug_line.fs"),
            new Shader("DebugP/debug_point_inst.vs", "DebugP/debug_point_inst.fs")
            //new Shader("DebugP/debug_point.vs", "DebugP/debug_point.fs")
        );
        debugPoints = new DebugPointDrawer();
        debugPoints->Init(new Shader("DebugP/backup/debug_point_circle.vs", "DebugP/backup/debug_point_circle.fs"));
    }


    void Init(Shader* lineShader, Shader* pointShader) {
        lineRenderer.Init(lineShader);
        pointRenderer.Init(pointShader);
    }

    // ================== Debug API ==================
    static inline void AddLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color = glm::vec3(1.0f)) {
        lineRenderer.AddLine(a, b, color);
    }

    static inline void AddWireframe(Mesh* mesh, const glm::mat4& transform, const glm::vec3& color = glm::vec3(1.0f)) {
        lineRenderer.AddWireframe(mesh, transform, color);
    }

    static inline void AddNormals(Mesh* mesh, const glm::mat4& transform, float length = 0.2f, const glm::vec3& color = glm::vec3(0, 1, 0)) {
        lineRenderer.AddNormals(mesh, transform, length, color);
    }

    static inline void AddPoint(const glm::vec3& pos, const glm::vec3& color = glm::vec3(1.0f), float size = 8.0f, DebugPointType type = DebugPointType::Square, bool o = true) {
        pointRenderer.AddPoint(pos, color, size, type, o);
    }

    // ================== System Update ==================
    void Update(float dt) override { 
        GEngine->renderer->frameScreen->Bind();

        // --- Draw ECS components ---
        auto entities = GEngine->scene->GetEntitiesWith<DebugMeshComponent>();
        for (auto& e : entities) {
            auto& comp = GEngine->scene->GetComponent<DebugMeshComponent>(e);
            if (comp.showWireframe) AddWireframe(GEngine->scene->GetComponent<MeshRenderer>(e).mesh, comp.transform, comp.color);
            if (comp.showNormals)   AddNormals(GEngine->scene->GetComponent<MeshRenderer>(e).mesh, comp.transform);
        }

        lineRenderer.Update(dt);
        pointRenderer.Update(dt);


        //Extra
        /*auto camera = GEngine->mainCamera;
        glm::mat4 mvp = camera->GetProjectionMatrix() * camera->GetViewMatrix();
        glEnable(GL_PROGRAM_POINT_SIZE);
        debugPoints->UploadPoints(pts); 
        debugPoints->Draw(mvp, debugPoints->debug_color, debugPoints->point_size);
        glDisable(GL_PROGRAM_POINT_SIZE);*/

        
        
        /*Scene& scene = *GEngine->scene; 
        glEnable(GL_PROGRAM_POINT_SIZE);
        if (DebugPointDrawer::show_debug_corners) {
            auto camera = GEngine->mainCamera;
            glm::mat4 mvp = camera->GetProjectionMatrix() * camera->GetViewMatrix();

            auto entitie = scene.GetFirstEntityOfType<SurfaceNetsComponent>();  
            auto& comp = scene.GetComponent<SurfaceNetsComponent>(entitie); 
            auto& corner = comp.buffer->debug_corners;  
            if (!corner.empty()) {
                std::vector<glm::vec3> contiguous(corner.begin(), corner.end());
                debugPoints->UploadPoints(contiguous);
                debugPoints->Draw(mvp, DebugPointDrawer::debug_color, DebugPointDrawer::point_size);
            } 
        }  
        glDisable(GL_PROGRAM_POINT_SIZE);*/


        //Final
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    static inline std::vector<glm::vec3> pts;
private:
    static inline DebugLineRenderer  lineRenderer;
    static inline DebugPointRenderer pointRenderer;


    static inline DebugPointDrawer* debugPoints;
};



//std::unordered_map<Entity, DebugMeshComponent> entityComponents;

// ================== ECS Component ==================
/*void AddEntityComponent(Entity e, DebugMeshComponent comp) {
    entityComponents[e] = comp;
}

void RemoveEntityComponent(Entity e) {
    entityComponents.erase(e);
}*/