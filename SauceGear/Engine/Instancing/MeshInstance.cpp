#include "MeshInstance.h"

//void MeshInstance::UploadInstanceMatrices( const std::vector<glm::mat4>& matrices ) {
//    uint32_t instanceVBO = VBO; //Teste
//
//    if (instanceVBO == 0)       
//        glGenBuffers(1, &instanceVBO);
//
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
//    glBufferData(
//        GL_ARRAY_BUFFER,
//        matrices.size() * sizeof(glm::mat4),
//        matrices.data(),
//        GL_DYNAMIC_DRAW
//    );
//
//    for (int i = 0; i < 4; i++) {
//        glEnableVertexAttribArray(5 + i);
//        glVertexAttribPointer(
//            5 + i,
//            4,
//            GL_FLOAT,
//            GL_FALSE,
//            sizeof(glm::mat4),
//            (void*)(sizeof(glm::vec4) * i)
//        );
//        glVertexAttribDivisor(5 + i, 1);
//    }
//
//    glBindVertexArray(0);
//}
//
//void MeshInstance::DrawSubmeshInstanced(uint32_t i, uint32_t count) {
//    const auto& sm = asset->submeshes[i];
//    glBindVertexArray(VAO);
//    glDrawElementsInstanced(
//        GL_TRIANGLES,
//        sm.indexCount,
//        GL_UNSIGNED_INT,
//        (void*)(sm.indexOffset * sizeof(uint32_t)),
//        count
//    );
//}


 
void MeshInstance::SetInstanceData(
    const void* data,
    size_t dataSize,
    const std::vector<std::pair<GLuint, GLint>>& attributes)
{
    if (instanceVBO == 0)
        glGenBuffers(1, &instanceVBO);

    mesh->Bind();
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_DYNAMIC_DRAW);

    // calcula stride
    size_t stride = 0;
    for (auto& a : attributes)
        stride += sizeof(float) * a.second;

    size_t offset = 0;
    for (auto& attr : attributes) {
        GLuint loc = attr.first;
        GLint  size = attr.second;

        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(
            loc, size, GL_FLOAT, GL_FALSE,
            stride, (void*)offset
        );
        glVertexAttribDivisor(loc, 1);

        offset += sizeof(float) * size;
    }

    glBindVertexArray(0);
}


void MeshInstance::DrawInstanced(uint32_t instanceCount, uint32_t submesh) const {
    const auto& sm = mesh->submeshes[submesh];

    mesh->Bind();
    glDrawElementsInstanced(
        GL_TRIANGLES,
        sm.indexCount,
        GL_UNSIGNED_INT,
        (void*)(sm.indexOffset * sizeof(uint32_t)),
        instanceCount
    );
}
