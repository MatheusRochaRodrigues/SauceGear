#pragma once 
#include <glad/glad.h>
#include <memory>
#include "../Assets/MeshAsset.h"

class MeshInstance {
public:
    explicit MeshInstance(std::shared_ptr<MeshAsset> mesh) : mesh(mesh) { }
     
    void Draw() const {
        mesh->Bind();

        for (size_t i = 0; i < mesh->submeshes.size(); ++i) {
            const auto& sm = mesh->submeshes[i]; 

            glDrawElements(
                GL_TRIANGLES,
                sm.indexCount,
                GL_UNSIGNED_INT,
                (void*)(sm.indexOffset * sizeof(uint32_t))
            );
        }
    }

    void DrawSubmesh(uint32_t i) const {
        const auto& sm = mesh->submeshes[i];

        mesh->Bind();
        glDrawElements(
            GL_TRIANGLES,
            sm.indexCount,
            GL_UNSIGNED_INT,
            (void*)(sm.indexOffset * sizeof(uint32_t))
        );

        /*glDrawElementsBaseVertex(
            GL_TRIANGLES,
            sm.indexCount,
            GL_UNSIGNED_INT,
            (void*)(sizeof(uint32_t) * sm.indexOffset),
            0
        );*/
    }

    void SetInstanceData( const void* data, size_t dataSize,
        const std::vector<std::pair<GLuint, GLint>>& attributes);

    void DrawInstanced(uint32_t instanceCount, uint32_t submesh = 0) const;

    std::shared_ptr<MeshAsset> mesh;
private:
    GLuint instanceVBO = 0;

    //Instance system
    /*void UploadInstanceMatrices(const std::vector<glm::mat4>& matrices);
    void DrawSubmeshInstanced(uint32_t i, uint32_t count);*/
};
