#pragma once
#include "../../Graphics/Mesh.h"
#include "../../Graphics/Shader.h"
#include "../ECS/System.h"
#include "../Core/Camera.h"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

struct DebugLine {
    glm::vec3 start;
    glm::vec3 end;
    glm::vec3 color;
};

class DebugLineRenderer : public System {
public:
    /*void Init(Shader* shader) {
        lineShader = shader;

        glGenVertexArrays(1, &lineVAO);
        glGenBuffers(1, &lineVBO);
        glBindVertexArray(lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glBindVertexArray(0);
    }*/

    //Instancing
    void Init(Shader* shader) {
        lineShader = shader;

        // VAO para a linha unitária
        glGenVertexArrays(1, &lineVAO);
        glBindVertexArray(lineVAO);

        // VBO com linha unitária (0,0,0) -> (1,0,0)
        GLfloat unitLine[] = { 0.0f,0.0f,0.0f, 1.0f,0.0f,0.0f };
        glGenBuffers(1, &unitVBO);
        glBindBuffer(GL_ARRAY_BUFFER, unitVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(unitLine), unitLine, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        // VBOs para instancing
        glGenBuffers(1, &startVBO);
        glGenBuffers(1, &endVBO);
        glGenBuffers(1, &colorVBO);

        // Ativa atributos de instancing
        glBindBuffer(GL_ARRAY_BUFFER, startVBO);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glVertexAttribDivisor(1, 1); // muda a cada instância

        glBindBuffer(GL_ARRAY_BUFFER, endVBO);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glVertexAttribDivisor(2, 1);

        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glVertexAttribDivisor(3, 1);

        glBindVertexArray(0);
    }

    // ================== Add lines ==================
    void AddLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color = glm::vec3(1.0f), bool persistent = false) {
        if (persistent) persistentLines.push_back({ a,b,color });
        else tempLines.push_back({ a,b,color });
    }

    void AddWireframe(Mesh* mesh, const glm::mat4& transform, const glm::vec3& color = glm::vec3(1.0f), bool persistent = false) {
        if (!mesh) return;
        for (size_t i = 0; i < mesh->indices.size(); i += 3) {
            glm::vec3 a = glm::vec3(transform * glm::vec4(mesh->vertices[mesh->indices[i]].Position, 1.0f));
            glm::vec3 b = glm::vec3(transform * glm::vec4(mesh->vertices[mesh->indices[i + 1]].Position, 1.0f));
            glm::vec3 c = glm::vec3(transform * glm::vec4(mesh->vertices[mesh->indices[i + 2]].Position, 1.0f));
            AddLine(a, b, color, persistent); AddLine(b, c, color, persistent); AddLine(c, a, color, persistent);
        }
    }

    void AddNormals(Mesh* mesh, const glm::mat4& transform, float length = 0.2f, const glm::vec3& color = glm::vec3(0, 1, 0), bool persistent = false) {
        if (!mesh) return;
        for (auto& v : mesh->vertices) {
            glm::vec3 pos = glm::vec3(transform * glm::vec4(v.Position, 1.0f));
            glm::vec3 tip = pos + glm::normalize(glm::vec3(transform * glm::vec4(v.Normal, 0.0f))) * length;
            AddLine(pos, tip, color, persistent);
        }
    }

    void Update(float dt) override {
        if (!lineShader || (tempLines.empty() && persistentLines.empty())) return;

        //instancing

        glm::mat4 vp = GEngine->mainCamera->GetProjectionMatrix() * GEngine->mainCamera->GetViewMatrix();

        glEnable(GL_LINE_SMOOTH);
        glLineWidth(1.5f);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);

        std::vector<DebugLine> allLines = persistentLines;
        allLines.insert(allLines.end(), tempLines.begin(), tempLines.end());

        // Prepara buffers instancing
        std::vector<glm::vec3> starts, ends, colors;
        starts.reserve(allLines.size());
        ends.reserve(allLines.size());
        colors.reserve(allLines.size());

        for (auto& l : allLines) {
            starts.push_back(l.start);
            ends.push_back(l.end);
            colors.push_back(l.color);
        }

        glBindVertexArray(lineVAO);

        glBindBuffer(GL_ARRAY_BUFFER, startVBO);
        glBufferData(GL_ARRAY_BUFFER, starts.size() * sizeof(glm::vec3), starts.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, endVBO);
        glBufferData(GL_ARRAY_BUFFER, ends.size() * sizeof(glm::vec3), ends.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_DYNAMIC_DRAW);

        lineShader->use();
        lineShader->setMat4("uVP", vp);

        // draw instanced: 2 vertices por linha, uma instância por linha
        glDrawArraysInstanced(GL_LINES, 0, 2, (GLsizei)allLines.size());

        glBindVertexArray(0);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glDisable(GL_LINE_SMOOTH);

        tempLines.clear(); // apenas linhas temporárias

    }

    void batchingLine() { }

    void perLine() {

        glm::mat4 vp = GEngine->mainCamera->GetProjectionMatrix() * GEngine->mainCamera->GetViewMatrix();

        glEnable(GL_LINE_SMOOTH);
        glLineWidth(1.5f);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);

        std::vector<DebugLine> allLines = persistentLines;
        allLines.insert(allLines.end(), tempLines.begin(), tempLines.end());

        std::vector<glm::vec3> vtx, colors;
        vtx.reserve(allLines.size() * 2);
        colors.reserve(allLines.size() * 2);
        for (auto& l : allLines) {
            vtx.push_back(l.start); vtx.push_back(l.end);
            colors.push_back(l.color); colors.push_back(l.color);
        }

        glBindVertexArray(lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferData(GL_ARRAY_BUFFER, vtx.size() * sizeof(glm::vec3), vtx.data(), GL_DYNAMIC_DRAW);

        lineShader->use();
        lineShader->setMat4("uVP", vp);

        for (size_t i = 0; i < vtx.size(); i += 2) {
            lineShader->setVec3("uColor", colors[i]);
            glDrawArrays(GL_LINES, i, 2);
        }

        glBindVertexArray(0);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glDisable(GL_LINE_SMOOTH);

        tempLines.clear(); // apenas linhas temporárias

    }

private:
    Shader* lineShader = nullptr;
    GLuint lineVAO = 0, lineVBO = 0;
    std::vector<DebugLine> tempLines;
    std::vector<DebugLine> persistentLines;


    GLuint unitVBO = 0;
    GLuint startVBO = 0, endVBO = 0, colorVBO = 0;
};
