#pragma once
#include "../ECS/System.h"
#include "../Core/Camera.h"
#include "../Graphics/Shader.h"
#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>

// =========================
// CPU representation (lógica)
// =========================
struct DebugLineCPU {
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 color;
};

// =========================
// GPU representation (vértice)
// =========================
struct DebugLineVertex {
    glm::vec3 pos;
    glm::vec3 color;
};

class DebugLineRenderer : public System {
public:
    void Init(Shader* shader) {
        lineShader = shader;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // posição
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE,
            sizeof(DebugLineVertex),
            (void*)offsetof(DebugLineVertex, pos)
        );

        // cor
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE,
            sizeof(DebugLineVertex),
            (void*)offsetof(DebugLineVertex, color)
        );

        glBindVertexArray(0);
    }

    void AddLine(
        const glm::vec3& a,
        const glm::vec3& b,
        const glm::vec3& color,
        bool persistent
    ) {
        if (persistent)
            persistentLines.push_back({ a, b, color });
        else
            tempLines.push_back({ a, b, color });
    }

    void Update(float) override {
        if (!lineShader) return;

        DrawBatch(persistentLines);
        DrawBatch(tempLines);

        tempLines.clear();
    }

private:
    void DrawBatch(const std::vector<DebugLineCPU>& lines) {
        if (lines.empty()) return;

        std::vector<DebugLineVertex> vertices;
        vertices.reserve(lines.size() * 2);

        for (const auto& l : lines) {
            vertices.push_back({ l.a, l.color });
            vertices.push_back({ l.b, l.color });
        }

        glm::mat4 vp = GEngine->mainCamera->GetProjectionMatrix() * GEngine->mainCamera->GetViewMatrix();

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(DebugLineVertex),
            vertices.data(),
            GL_DYNAMIC_DRAW
        );

        glBindVertexArray(vao);
        lineShader->use();
        lineShader->setMat4("uVP", vp);
        glDrawArrays(GL_LINES, 0, (GLsizei)vertices.size());
        glBindVertexArray(0);
    }

private:
    Shader* lineShader = nullptr;
    GLuint vao = 0;
    GLuint vbo = 0;

    std::vector<DebugLineCPU> persistentLines;
    std::vector<DebugLineCPU> tempLines;
};
