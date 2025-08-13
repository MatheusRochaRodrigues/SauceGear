#include "Mesh.h" 

// constructor
Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, Material* material)
{
    this->vertices = vertices;
    this->indices = indices;
    this->material = material;

    // now that we have all the required data, set the vertex buffers and its attribute pointers.
    setupMesh();
}

Mesh::Mesh(bool ,vector<Vertex> vertices, vector<unsigned int> indices, Material* material)
{
    this->vertices = vertices;
    this->indices = indices;
    this->material = material; 
}

// render the mesh
void Mesh::Draw() const
{
    //// bind appropriate textures
    //unsigned int diffuseNr = 1;
    //unsigned int specularNr = 1;
    //unsigned int normalNr = 1;
    //unsigned int heightNr = 1;
    //for (unsigned int i = 0; i < textures.size(); i++)
    //{
    //    glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
    //    // retrieve texture number (the N in diffuse_textureN)
    //    string number;
    //    string name = textures[i].type;
    //    if (name == "texture_diffuse")
    //        number = std::to_string(diffuseNr++);
    //    else if (name == "texture_specular")
    //        number = std::to_string(specularNr++); // transfer unsigned int to string
    //    else if (name == "texture_normal")
    //        number = std::to_string(normalNr++); // transfer unsigned int to string
    //    else if (name == "texture_height")
    //        number = std::to_string(heightNr++); // transfer unsigned int to string

    //    // now set the sampler to the correct texture unit
    //    glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
    //    // and finally bind the texture
    //    glBindTexture(GL_TEXTURE_2D, textures[i].ID);
    //}

    /*if (material)
        material->Bind();*/

    // draw mesh
    // bind VAO/VBO e draw elements
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
} 

// initializes all the buffer objects/arrays
void Mesh::setupMesh()
{
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
    glBindVertexArray(0);
}


//void Mesh::setupMesh() {
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    glGenBuffers(1, &EBO);
//
//    glBindVertexArray(VAO);
//
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
//
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
//
//    // Vertex Positions
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
//
//    // Vertex Normals
//    glEnableVertexAttribArray(1);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
//
//    // Vertex Texture Coords
//    glEnableVertexAttribArray(2);
//    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
//
//    // Tangents
//    glEnableVertexAttribArray(3);
//    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
//
//    // Bitangents
//    glEnableVertexAttribArray(4);
//    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
//
//    // Vertex Colors
//    glEnableVertexAttribArray(5);
//    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, VertexColor));
//
//    // Bone IDs
//    glEnableVertexAttribArray(6);
//    glVertexAttribIPointer(6, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
//
//    // Weights
//    glEnableVertexAttribArray(7);
//    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
//
//    glBindVertexArray(0);
//}




void Mesh::SetData(
    const std::vector<Vertex>& newVertices,
    const std::vector<unsigned int>& newIndices,
    Material* newMaterial) 
{
    vertices = newVertices;
    indices = newIndices;
    material = newMaterial;
    setupMesh();
}



void Mesh::BuildFromRawData(const std::vector<float>& rawData, int stride) {
    vertices.clear();
    indices.clear();

    // Cada vértice terá `stride` elementos (ex: 8 para pos(3) + normal(3) + uv(2))
    for (size_t i = 0; i < rawData.size(); i += stride) {
        Vertex v{};
        v.Position = glm::vec3(rawData[i], rawData[i + 1], rawData[i + 2]);
        v.Normal = glm::vec3(rawData[i + 3], rawData[i + 4], rawData[i + 5]);
        v.TexCoords = glm::vec2(rawData[i + 6], rawData[i + 7]);

        vertices.push_back(v);
        indices.push_back(static_cast<unsigned int>(indices.size())); // índice sequencial
    }

    setupMesh();
}


//spehere instancing
//glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
//glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(LightInstanceData), instanceData.data(), GL_STATIC_DRAW);
void Mesh::SetInstanceData(const void* data, size_t dataSize, const std::vector<std::pair<GLuint, GLint>>& attributes) {
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

void Mesh::DrawInstanced(GLsizei count) const {
    glBindVertexArray(VAO); 
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0, count);
    glBindVertexArray(0);
}
