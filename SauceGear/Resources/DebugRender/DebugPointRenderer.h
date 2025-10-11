#pragma once
#include "../../Graphics/Shader.h"
#include "../ECS/System.h"
#include <glm/glm.hpp>
#include <vector>

enum class DebugPointType { Square, Circle };

struct DebugPoint {
    glm::vec3 pos;
    glm::vec3 color;
    float size;
    DebugPointType type = DebugPointType::Square;
};

class DebugPointRenderer : public System {
public:
    void Init(Shader* shader) {
        pointShader = shader;

        // VBO de geometria de um ponto unitário (um único vértice na origem)
        glGenVertexArrays(1, &pointVAO);
        glGenBuffers(1, &pointVBO);
        glBindVertexArray(pointVAO);
        glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
        glm::vec3 origin(0.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(origin), &origin, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        // VBO de instâncias (posição, cor, tamanho, tipo)
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        // posição (vec3) + cor (vec3) + tamanho (float) + tipo (float)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugPoint), (void*)offsetof(DebugPoint, pos));
        glVertexAttribDivisor(1, 1);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(DebugPoint), (void*)offsetof(DebugPoint, color));
        glVertexAttribDivisor(2, 1);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(DebugPoint), (void*)offsetof(DebugPoint, size));
        glVertexAttribDivisor(3, 1);

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(DebugPoint), (void*)offsetof(DebugPoint, type));
        glVertexAttribDivisor(4, 1);

        glBindVertexArray(0);
    }

    void AddPoint(const glm::vec3& pos, const glm::vec3& color = glm::vec3(1.0f),
        float size = 6.0f, DebugPointType type = DebugPointType::Square, bool persistent = false)
    {
        if (persistent) persistentPoints.push_back({ pos,color,size,type });
        else tempPoints.push_back({ pos,color,size,type });
    }

    void Update(float dt) override {
        if (!pointShader || (tempPoints.empty() && persistentPoints.empty())) return;

        glm::mat4 vp = GEngine->mainCamera->GetProjectionMatrix() * GEngine->mainCamera->GetViewMatrix();

        std::vector<DebugPoint> allPoints = persistentPoints;
        allPoints.insert(allPoints.end(), tempPoints.begin(), tempPoints.end());

        glEnable(GL_PROGRAM_POINT_SIZE);

        // envia dados de instância para GPU
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, allPoints.size() * sizeof(DebugPoint), allPoints.data(), GL_DYNAMIC_DRAW);

        pointShader->use();
        pointShader->setMat4("uVP", vp);

        glBindVertexArray(pointVAO);
        glDrawArraysInstanced(GL_POINTS, 0, 1, static_cast<GLsizei>(allPoints.size()));
        glBindVertexArray(0);

        glDisable(GL_PROGRAM_POINT_SIZE);

        tempPoints.clear();
    }

private:
    Shader* pointShader = nullptr;
    GLuint pointVAO = 0, pointVBO = 0, instanceVBO = 0;

    std::vector<DebugPoint> tempPoints;
    std::vector<DebugPoint> persistentPoints;
};
