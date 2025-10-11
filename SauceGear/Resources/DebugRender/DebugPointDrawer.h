#pragma once 
#include "../Graphics/Mesh.h" 

// Gerencie isso em algum DebugRenderer singleton/class (ex.: DebugPoints)
struct DebugPointDrawer {

    void Init(Shader* s) {
        shader = s;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindVertexArray(0);
    }

    // atualiza com novos pontos (chamar por frame, antes de Draw)
    void UploadPoints(const std::vector<glm::vec3>& pts) {
        if (pts.empty()) return; 
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        size_t needed = pts.size() * sizeof(glm::vec3);
        if (needed > capacity) {
            // alocar novo buffer
            glBufferData(GL_ARRAY_BUFFER, needed, pts.data(), GL_DYNAMIC_DRAW);
            capacity = needed;
        }
        else {
            // apenas subdata
            glBufferSubData(GL_ARRAY_BUFFER, 0, needed, pts.data());
        }
    }

    void Draw(const glm::mat4& mvp, const glm::vec3& color, float pointSize) {
        if (!shader) return;
        shader->use();
        shader->setMat4("uMVP", mvp);
        shader->setVec3("uColor", color);
        shader->setFloat("uPointSize", pointSize);

        glBindVertexArray(vao);
        // assumindo que VBO tem N pontos (capacidade pode ser maior — você precisa saber a contagem)
        // eu recomendo manter contagem externa; aqui só um exemplo assume você salvou pts_count.
        glDrawArrays(GL_POINTS, 0, int(capacity / sizeof(glm::vec3))); // NÃO ideal: prefira passar count real
        glBindVertexArray(0); 
    }


    static inline bool show_debug_corners = true;
    static inline float point_size = 6.0f;
    static inline glm::vec3 debug_color = glm::vec3(1.0f, 0.0f, 0.0f);

private:

    GLuint vao = 0, vbo = 0;
    size_t capacity = 0;
    Shader* shader = nullptr; // carrega debug_point.vert/frag por ex.

    size_t pointsCount = 0; // membro da classe
};



//header
// DebugPointsRenderer* debugPoints;

//void PBRPipeline::Init() { 

/*
// ============== Debug points init ==============
debugPoints = new DebugPointsRenderer();
debugPoints->Init(new Shader("DebugP/debug_point.vs", "DebugP/debug_point.fs"));
*/



//Forward pass antes de drawSkyBox

/*
glEnable(GL_PROGRAM_POINT_SIZE);
if (DebugPointsRenderer::show_debug_corners) {
    auto camera = GEngine->mainCamera;
    glm::mat4 mvp = camera->GetProjectionMatrix() * camera->GetViewMatrix();

    auto entitie = scene.GetFirstEntityOfType<SurfaceNetsComponent>();
    std::cout << "Entity id: " << (int)entitie << std::endl;

    if (!scene.HasComponent<SurfaceNetsComponent>(entitie)) {
        std::cout << "⚠️ Entity não tem SurfaceNetsComponent!" << std::endl;
        return;
    }

    auto& comp = scene.GetComponent<SurfaceNetsComponent>(entitie);
    if (!comp.buffer) {
        std::cout << "⚠️ comp.buffer é nullptr!" << std::endl;
        return;
    }

    auto& corner = comp.buffer->debug_corners;
    std::cout << "Corners count: " << corner.size() << std::endl;

    if (!corner.empty()) {
        std::vector<glm::vec3> contiguous(corner.begin(), corner.end());
        debugPoints->UploadPoints(contiguous);
        debugPoints->Draw(mvp, DebugPointsRenderer::debug_color, DebugPointsRenderer::point_size);
    }
    else {
        std::cout << "⚠️ Nenhum corner encontrado!" << std::endl;
    }
}

std::cout << "tes 2" << std::endl;

std::cout << "tes 2" << std::endl;
glDisable(GL_PROGRAM_POINT_SIZE);
*/