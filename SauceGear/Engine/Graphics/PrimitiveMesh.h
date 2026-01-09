//#ifndef MESH_H
//#define MESH_H    
//#include "Vertex.h"  
//#include "../Assets/MaterialAsset.h"  
//
//using namespace std; 
// 
//struct SubMeshLegacy {
//    // faixa de índices dentro do EBO
//    uint32_t indexOffset = 0;   // em elementos (não bytes)
//    uint32_t indexCount = 0; 
//
//    /// just only reference, to know wich material has this submesh
//    std::shared_ptr<MaterialInstance> materialAsset; 
//    //std::shared_ptr<MaterialAsset> materialAsset; 
//    //uint32_t materialSlot; // index ou id
//    std::string name;
//};
//
//class MeshLegacy {
//public: 
//    string name;
//    // mesh Data
//    GLuint                VAO = 0;
//    vector<Vertex>        vertices;
//    std::vector<uint32_t> indices;
//    std::vector<SubMeshLegacy>  submeshes;
//    std::vector<MeshLegacy*>    children;        // hierarquia local
//    std::string           directory;       // diretório do asset 
//
//    MeshLegacy() = default;
//     
//    MeshLegacy(const std::vector<Vertex>& v, const std::vector<uint32_t>& i) {
//        vertices = v; indices = i; submeshes.push_back(SubMeshLegacy{ 0, (uint32_t)i.size()});
//        setupMesh();
//    }
//
//    MeshLegacy(const std::vector<Vertex>& v, const std::vector<uint32_t>& i, const std::vector<SubMeshLegacy>& sms) {
//        vertices = v; indices = i; submeshes = sms; 
//        setupMesh();
//    }
//
//    MeshLegacy(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<uint32_t>& inds) {
//        UploadFromRaw(positions, normals, inds);
//    }
//
//    void Set(const std::vector<Vertex>& v,  const std::vector<uint32_t>& i, const std::vector<SubMeshLegacy>& sms) {
//        vertices = v; indices = i; submeshes = sms;
//        setupMesh();
//    }
//
//
//    void Draw() const {             //AllSubmesh
//        glBindVertexArray(VAO);
//        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
//        glBindVertexArray(0);
//    }
//    
//    void DrawSubmesh(uint32_t i /*subIndex*/) const {
//        if (VAO == 0 || indices.empty() || submeshes.empty()) return;
//        if (i >= submeshes.size()) return;
//
//        const auto& sm = submeshes[i];
//        glDrawElements(
//            GL_TRIANGLES,
//            sm.indexCount,
//            GL_UNSIGNED_INT,
//            (void*)(sm.indexOffset * sizeof(uint32_t))
//        );
//    }
//
//    void AddChild(MeshLegacy* child) { children.push_back(child); }
//
//
//    //---------------------------------------------------------------------------------------
//    //------------------------------------- RAW ---------------------------------------------
//    //---------------------------------------------------------------------------------------
//    // Novo método para criar vertices e enviar para GPU
//    void UploadFromRaw(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<uint32_t>& inds ) {
//        vertices.clear(); 
//        indices = inds; 
//
//        vertices.reserve(positions.size()); 
//        for (size_t i = 0; i < positions.size(); ++i) {
//            Vertex v{};
//            v.Position = positions[i];
//            v.Normal = (i < normals.size()) ? normals[i] : glm::vec3(0.0f);
//            v.TexCoords = glm::vec2(0.0f);  // opcional, sem uv
//            v.Tangent = glm::vec3(0.0f);
//            v.Bitangent = glm::vec3(0.0f);
//            vertices.push_back(v);
//        }
//        submeshes.push_back(SubMeshLegacy{ 0, static_cast<uint32_t>(inds.size())}); 
//        setupMesh();    // setup da GPU
//    }
//
//    template <typename VecType>
//    void UploadFromRaw(const std::vector<VecType>& positions, const std::vector<VecType>& normals, const std::vector<uint32_t>& inds, std::shared_ptr<MaterialInstance> m = nullptr)
//    {
//        vertices.clear();
//        vertices.reserve(positions.size()); 
//         
//        for (size_t i = 0; i < positions.size(); ++i) {
//            Vertex v{};
//            // extrair apenas xyz, mesmo se for vec4
//            v.Position = glm::vec3(positions[i]);
//            v.Normal = (i < normals.size()) ? glm::vec3(normals[i]) : glm::vec3(0.0f);
//            v.TexCoords = glm::vec2(0.0f);
//            v.Tangent = glm::vec3(0.0f);
//            v.Bitangent = glm::vec3(0.0f);
//            vertices.push_back(v);
//        }
//
//        indices = std::move(inds);
//
//        submeshes.push_back(SubMeshLegacy{ 0, (uint32_t)inds.size(), m });
//        setupMesh();
//    }
//
//
//    //---------------------------------------------------------------------------------------
//    //------------------------------------- Instancing --------------------------------------
//    //------------------------------------------------=--------------------------------------
//    void SetInstanceData(const void* data, size_t dataSize, const std::vector<std::pair<GLuint, GLint>>& attributes) {
//        if (instanceVBO == 0)
//            glGenBuffers(1, &instanceVBO);
//
//        glBindVertexArray(VAO);
//        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
//        glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
//
//        size_t stride = 0;
//        for (const auto& attr : attributes) {
//            stride += sizeof(float) * attr.second;
//        }
//
//        size_t offset = 0;
//        for (const auto& attr : attributes) {
//            GLuint loc = attr.first;
//            GLint size = attr.second;
//
//            glEnableVertexAttribArray(loc);
//            glVertexAttribPointer(loc, size, GL_FLOAT, GL_FALSE, stride, (void*)offset);
//            glVertexAttribDivisor(loc, 1); // este atributo muda por instância
//
//            offset += sizeof(float) * size;
//        }
//
//        glBindVertexArray(0);
//    }
//
//    void DrawInstanced(GLsizei count, uint32_t subIndex = 0) const {
//        if (subIndex >= submeshes.size()) return;
//        const auto& sm = submeshes[subIndex];
//        glBindVertexArray(VAO);
//        glDrawElementsInstanced(GL_TRIANGLES, sm.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm.indexOffset), count);
//        glBindVertexArray(0);
//    }
//         
//private:
//    // render data 
//    GLuint VBO = 0;
//    GLuint EBO = 0;
//
//    //Instancing
//    GLuint instanceVBO = 0;
//    GLsizei instanceCount = 0; 
//     
//    // initializes all the buffer objects/arrays
//    void setupMesh() {
//        if (VAO == 0) glGenVertexArrays(1, &VAO);
//        if (VBO == 0) glGenBuffers(1, &VBO);
//        if (EBO == 0) glGenBuffers(1, &EBO);
//
//        glBindVertexArray(VAO);
//
//        glBindBuffer(GL_ARRAY_BUFFER, VBO);
//        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
//
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
//
//        // layout
//        size_t stride = sizeof(Vertex);
//        // pos
//        glEnableVertexAttribArray(0);
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, Position));
//        // normal
//        glEnableVertexAttribArray(1);
//        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, Normal));
//        // uv
//        glEnableVertexAttribArray(2);
//        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, TexCoords));
//        // tangent
//        glEnableVertexAttribArray(3);
//        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, Tangent));
//        // bitangent
//        glEnableVertexAttribArray(4);
//        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, Bitangent));
//
//        glBindVertexArray(0);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//
//    }
//};
//#endif