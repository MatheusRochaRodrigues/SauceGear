#ifndef MESH_H
#define MESH_H
   
#include "../Scene/Components/Material.h"                     

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

class Mesh {
public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    Material*            material;                          
    GLuint               VAO;
      
    //Mesh(vector<Vertex> vertices, vector<unsigned int> indices);
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, Material* material);

    //Mesh(bool, vector<Vertex> vertices, vector<unsigned int> indices);
    Mesh(bool,vector<Vertex> vertices, vector<unsigned int> indices, Material* material);

    Mesh() = default;

    // render the mesh 
    void Draw() const;      //(Shader& shader)

    void DrawInstanced(GLsizei count) const;
    void SetInstanceData(const void* data, size_t dataSize, const std::vector<std::pair<GLuint, GLint>>& attributes);
     

    void SetData(const std::vector<Vertex>& newVertices,
        const std::vector<unsigned int>& newIndices,
        Material* newMaterial = nullptr);
     
    // Novo: função utilitária para montar a malha a partir de floats crus
    void BuildFromRawData(const std::vector<float>& rawData, int stride);
private:
    // render data 
    GLuint VBO, EBO;

    //Instancing
    GLuint instanceVBO = 0;
    GLsizei instanceCount = 0;

    // initializes all the buffer objects/arrays
    void setupMesh();
};
#endif