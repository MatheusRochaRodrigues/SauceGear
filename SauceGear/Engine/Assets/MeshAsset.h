#pragma once
#include <vector>
#include <string>
#include <memory>
#include <glad/glad.h>
#include "../Graphics/Vertex.h"
#include "../Assets/MaterialAsset.h"

struct SubMesh {
    uint32_t indexOffset = 0;
    uint32_t indexCount = 0;
    std::shared_ptr<MaterialAsset> materialAsset;
    std::string name;
};

class MeshAsset {
public:
    std::string name;

    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
    std::vector<SubMesh>  submeshes; 

    MeshAsset() = default;
      
    MeshAsset(std::vector<Vertex> verts, std::vector<uint32_t> inds, std::vector<SubMesh> subm = {}) {
        SetData(std::move(verts), std::move(inds), std::move(subm));
    }

    void SetData( std::vector<Vertex> verts, std::vector<uint32_t> inds, std::vector<SubMesh> subm = {}) {
        vertices = std::move(verts);
        indices = std::move(inds);

        if (subm.empty()) BuildDefaultSubmesh();    // criar submesh padrão  
        else submeshes = std::move(subm);

        UploadToGPU();
    }

    void ReloadFrom(const MeshAsset& src) {
        vertices = src.vertices;
        indices = src.indices;
        submeshes = src.submeshes;

        UploadToGPU(); //  hot-reload  
    }

    void Bind() const {
        glBindVertexArray(VAO);
    }

private:
    // GPU
    GLuint VAO = 0; GLuint VBO = 0; GLuint EBO = 0;

    void BuildDefaultSubmesh() {
        submeshes.clear();
        if (indices.empty()) return;

        SubMesh sm;
        sm.indexOffset = 0;
        sm.indexCount = static_cast<uint32_t>(indices.size());
        submeshes.push_back(sm);
    }

    void UploadToGPU() {
        if (VAO == 0) {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);
        }

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Vertex),
            vertices.data(),
            GL_STATIC_DRAW
        );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(uint32_t),
            indices.data(),
            GL_STATIC_DRAW
        );

        // layout
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
    }


public:
    // Novo método para criar vertices e enviar para GPU
    void UploadFromRaw(const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec3>& normals,
        const std::vector<uint32_t>& inds)
    {
        vertices.clear();
        indices = inds;

        vertices.reserve(positions.size());
        for (size_t i = 0; i < positions.size(); ++i) {
            Vertex v{};
            v.Position = positions[i];
            v.Normal = (i < normals.size()) ? normals[i] : glm::vec3(0.0f);
            v.TexCoords = glm::vec2(0.0f);  // opcional, sem uv
            v.Tangent = glm::vec3(0.0f);
            v.Bitangent = glm::vec3(0.0f);
            vertices.push_back(v);
        }
        submeshes.push_back(SubMesh{ 0, static_cast<uint32_t>(inds.size()) });

        // setupMesh
        UploadToGPU();
    }
     
    template <typename VecType>
    void UploadFromRaw(const std::vector<VecType>& positions,
        const std::vector<VecType>& normals,
        const std::vector<uint32_t>& inds)
    {
        vertices.clear();
        vertices.reserve(positions.size());

        for (size_t i = 0; i < positions.size(); ++i) {
            Vertex v{};
            // extrair apenas xyz, mesmo se for vec4
            v.Position = glm::vec3(positions[i]);
            v.Normal = (i < normals.size()) ? glm::vec3(normals[i]) : glm::vec3(0.0f);
            v.TexCoords = glm::vec2(0.0f);
            v.Tangent = glm::vec3(0.0f);
            v.Bitangent = glm::vec3(0.0f);
            vertices.push_back(v);
        }

        indices = std::move(inds);

        submeshes.push_back(SubMesh{ 0, (uint32_t)inds.size() });
        UploadToGPU();
    }
};
