#pragma once
#include "../ECS/System.h"
#include "../Graphics/Shader.h"
#include "../Core/Camera.h"
#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>

enum class DebugPointType : uint8_t {
    Square = 0,
    Circle = 1
};

struct DebugPoint {
    glm::vec3 pos;
    glm::vec3 color;
    float size;   // em pixels
    float type;   // 0.0 = square, 1.0 = circle
};


class DebugPointRenderer : public System {
public:
    void Init(Shader* shader) {
        pointShader = shader;

        // Quad unitário (NDC local)
        float quad[8] = {
            -1.f, -1.f,
             1.f, -1.f,
             1.f,  1.f,
            -1.f,  1.f
        };

        uint32_t indices[6] = { 0,1,2, 2,3,0 };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vboQuad);
        glBindBuffer(GL_ARRAY_BUFFER, vboQuad);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        // pos
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugPoint), (void*)offsetof(DebugPoint, pos));
        glVertexAttribDivisor(1, 1);

        // color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(DebugPoint), (void*)offsetof(DebugPoint, color));
        glVertexAttribDivisor(2, 1);

        // size
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(DebugPoint), (void*)offsetof(DebugPoint, size));
        glVertexAttribDivisor(3, 1);

        // type
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(DebugPoint), (void*)offsetof(DebugPoint, type));
        glVertexAttribDivisor(4, 1);

        glBindVertexArray(0);
    }

    void AddPoint(const glm::vec3& pos,
        const glm::vec3& color,
        float size,
        DebugPointType type,
        bool persistent = false)
    {
        DebugPoint p;
        p.pos = pos;
        p.color = color;
        p.size = size;
        p.type = (type == DebugPointType::Circle) ? 1.0f : 0.0f; 

        if (persistent)
            persistentPoints.push_back(p);
        else
            tempPoints.push_back(p);
    }

    void Update(float) override {
        DrawBatch(persistentPoints);
        DrawBatch(tempPoints);
        tempPoints.clear();
    }

private:
    void DrawBatch(const std::vector<DebugPoint>& points) {
        if (points.empty()) return; 
        auto* cam = GEngine->mainCamera; 

        glm::mat4 vp = cam->GetProjectionMatrix() * cam->GetViewMatrix();

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(DebugPoint), points.data(), GL_DYNAMIC_DRAW);

        glBindVertexArray(vao);
        pointShader->use();
        pointShader->setMat4("uVP", vp);
        pointShader->setVec2("uViewport", glm::vec2(GEngine->window->GetWidth(), GEngine->window->GetHeight()));

        //upload all points once
        //draw all points in 1 call
        glDrawElementsInstanced(
            GL_TRIANGLES,
            6,              //QUANTOS vértices (índices) POR instância
            GL_UNSIGNED_INT,
            0,
            (GLsizei)points.size()      //quantas instâncias desse modelo
        );

        glBindVertexArray(0);
    }

private:
    Shader* pointShader = nullptr;

    GLuint vao = 0;
    GLuint vboQuad = 0;
    GLuint ebo = 0;
    GLuint instanceVBO = 0;

    std::vector<DebugPoint> persistentPoints;
    std::vector<DebugPoint> tempPoints;
};
