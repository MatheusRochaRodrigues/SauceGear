#ifndef MESH_H
#define MESH_H

#include "../Resources/DefineMaterials/PBRMaterial.h"    
#include "../Resources/DefineMaterials/MaterialInstance.h"         

using namespace std;

#define MAX_BONE_INFLUENCE 4 

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;

    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;

    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct SubMesh {
    // faixa de índices dentro do EBO
    uint32_t indexOffset = 0;   // em elementos (não bytes)
    uint32_t indexCount = 0;
    // material “default” associado a esse submesh (pode ser sobrescrito no MeshRenderer)
    std::shared_ptr<MaterialInstance> material;
};

class Mesh {
public: 
    string name;
    // mesh Data
    vector<Vertex>        vertices;
    std::vector<uint32_t> indices;
    std::vector<SubMesh>  submeshes;
    std::vector<Mesh*>    children;        // hierarquia local
    std::string           directory;       // diretório do asset
    GLuint                VAO = 0;

    Mesh() = default;
     
    Mesh(const std::vector<Vertex>& v, const std::vector<uint32_t>& i, std::shared_ptr<MaterialInstance> m = nullptr) {
        vertices = v;
        indices = i;
        submeshes.push_back(SubMesh{ 0, (uint32_t)i.size(), m });
        setupMesh();
    }

    Mesh(const std::vector<Vertex>& v, const std::vector<uint32_t>& i, const std::vector<SubMesh>& sms, std::shared_ptr<MaterialInstance> m = nullptr) {
        vertices = v;
        indices = i;
        submeshes = sms; 
        setupMesh();
    }

    void Set(const std::vector<Vertex>& v,  const std::vector<uint32_t>& i, const std::vector<SubMesh>& sms) {
        vertices = v;
        indices = i;
        submeshes = sms;
        setupMesh();
    }

    void Set(const std::vector<Vertex>& v,const std::vector<uint32_t>& i, std::shared_ptr<MaterialInstance> m = nullptr) {
        vertices = v;
        indices = i;
        submeshes.push_back(SubMesh{ 0, (uint32_t)i.size(), m });
        setupMesh();
    }

    void AddChild(Mesh* child) { children.push_back(child); }

    //Drawing
    void DrawSubmesh(uint32_t subIndex) const {
        if (subIndex >= submeshes.size()) return;
        const auto& sm = submeshes[subIndex];
        // bind VAO/VBO e draw elements
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, sm.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm.indexOffset));
        glBindVertexArray(0);
    } 

    void Draw() const {       //AllSubmesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    //Render
    void Render() const {
        // desenha todos os submeshes com seus materiais “default” (fallback)
        glBindVertexArray(VAO);
        for (const auto& sm : submeshes) {
            if (sm.material) sm.material->Bind();
            glDrawElements(GL_TRIANGLES, sm.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm.indexOffset));
        }
        glBindVertexArray(0);
    }

    void RenderAll() const {
        glBindVertexArray(VAO);
        for (auto& sm : submeshes) {
            if (sm.material) sm.material->Bind();
            glDrawElements(GL_TRIANGLES, sm.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm.indexOffset));
        }
        glBindVertexArray(0);

        for (auto* child : children) if (child) child->RenderAll();
    }

    // initializes all the buffer objects/arrays
    void setupMesh() {
        if (VAO == 0) glGenVertexArrays(1, &VAO);
        if (VBO == 0) glGenBuffers(1, &VBO);
        if (EBO == 0) glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

        // layout
        size_t stride = sizeof(Vertex);
        // pos
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, Position));
        // normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, Normal));
        // uv
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, TexCoords));
        // tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, Tangent));
        // bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    }

    //Instancing
    void SetInstanceData(const void* data, size_t dataSize, const std::vector<std::pair<GLuint, GLint>>& attributes) {
        if (instanceVBO == 0)
            glGenBuffers(1, &instanceVBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);

        size_t stride = 0;
        for (const auto& attr : attributes) {
            stride += sizeof(float) * attr.second;
        }

        size_t offset = 0;
        for (const auto& attr : attributes) {
            GLuint loc = attr.first;
            GLint size = attr.second;

            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, size, GL_FLOAT, GL_FALSE, stride, (void*)offset);
            glVertexAttribDivisor(loc, 1); // este atributo muda por instância

            offset += sizeof(float) * size;
        }

        glBindVertexArray(0);
    }

    void DrawInstanced(GLsizei count, uint32_t subIndex = 0) const {
        if (subIndex >= submeshes.size()) return;
        const auto& sm = submeshes[subIndex];
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, sm.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm.indexOffset), count);
        glBindVertexArray(0);
    }



    // Novo construtor: recebe posições, normais e índices diretamente
    Mesh(const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec3>& normals,
        const std::vector<uint32_t>& inds)
    {
        UploadFromRaw(positions, normals, inds);
    }

    // Novo método para criar vertices e enviar para GPU
    void UploadFromRaw(const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec3>& normals,
        const std::vector<uint32_t>& inds,
        std::shared_ptr<MaterialInstance> m = nullptr)
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
        submeshes.push_back(SubMesh{ 0, static_cast<uint32_t>(inds.size()), m });
        // setup da GPU
        setupMesh();
    }

    template <typename VecType>
    void UploadFromRaw(const std::vector<VecType>& positions,
        const std::vector<VecType>& normals,
        const std::vector<uint32_t>& inds,
        std::shared_ptr<MaterialInstance> m = nullptr)
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

        submeshes.push_back(SubMesh{ 0, (uint32_t)inds.size(), m });
        setupMesh();
    }

         
private:
    // render data 
    GLuint VBO = 0;
    GLuint EBO = 0;

    //Instancing
    GLuint instanceVBO = 0;
    GLsizei instanceCount = 0; 
};
#endif