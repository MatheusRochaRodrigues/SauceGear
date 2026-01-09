#pragma once 
#include "../Instancing/MeshInstance.h"
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

struct CachedWireframe {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei vertexCount = 0;
};

class WireframeCache {
public:
    static CachedWireframe* Get(MeshInstance* mesh) {
        auto it = cache.find(mesh);
        if (it != cache.end())
            return &it->second;

        auto result = cache.emplace(mesh, Build(mesh));
        return &result.first->second;
    }

private:
    static inline std::unordered_map<MeshInstance*, CachedWireframe> cache;

    static CachedWireframe Build(MeshInstance* meshI) {
        auto& mesh = meshI->mesh;
        CachedWireframe wf;

        std::vector<glm::vec3> lines;
        lines.reserve(mesh->indices.size() * 2);

        for (size_t i = 0; i < mesh->indices.size(); i += 3) {
            const auto& a = mesh->vertices[mesh->indices[i]].Position;
            const auto& b = mesh->vertices[mesh->indices[i + 1]].Position;
            const auto& c = mesh->vertices[mesh->indices[i + 2]].Position;

            lines.push_back(a); lines.push_back(b);
            lines.push_back(b); lines.push_back(c);
            lines.push_back(c); lines.push_back(a);
        }

        glGenVertexArrays(1, &wf.vao);
        glGenBuffers(1, &wf.vbo);

        glBindVertexArray(wf.vao);
        glBindBuffer(GL_ARRAY_BUFFER, wf.vbo);
        glBufferData(GL_ARRAY_BUFFER,
            lines.size() * sizeof(glm::vec3),
            lines.data(),
            GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

        glBindVertexArray(0);

        wf.vertexCount = static_cast<GLsizei>(lines.size());
        return wf;
    }
};
