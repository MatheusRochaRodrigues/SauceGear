#pragma once
#include <unordered_map>
#include <vector>
#include "../../Scene/Components/Material.h"
#include "../../Scene/Components/MeshFilter.h"

struct MeshRenderer {
    Mesh* mesh = nullptr;
    std::unordered_map<Material*, std::vector<SubMesh*>> batches;

    MeshRenderer() = default;

    MeshRenderer(Mesh* m) { SetMesh(m); }

    void SetMesh(Mesh* m) {
        mesh = m;
        RebuildBatches();
    }
     
    void RebuildBatches() {
        batches.clear();
        if (!mesh) return;
        for (auto& sm : mesh->submeshes) {
            if (sm.material == nullptr) std::cout << "mapa defult created  " << mesh->name << "    " << mesh->submeshes.size() << std::endl;
            sm.material = (sm.material != nullptr) ? sm.material : MaterialDefaults::Get();
             
            batches[sm.material].push_back(&sm);
        }
    }

    void Draw() {
        if (!mesh) return;
        for (auto& [mat, sms] : batches) {
            if (!mat) continue; 
            for (auto* sm : sms) {
                glBindVertexArray(mesh->VAO);
                glDrawElements(GL_TRIANGLES, sm->indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm->indexOffset));
            }
        }
        glBindVertexArray(0);
    }

    void Draw(Material* mat) {
        if (!mesh) return;
        if (!mat) return;
        for (auto& sm : batches[mat]) { 
            glBindVertexArray(mesh->VAO);
            glDrawElements(GL_TRIANGLES, sm->indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm->indexOffset)); 
        }
        glBindVertexArray(0);
    }

    void Paint() {
        if (!mesh) return;
        for (auto& [mat, sms] : batches) {
            if (!mat) continue;
            mat->Bind();
            //shader->setMat4("model", trans.GetMatrix());
            for (auto* sm : sms) {
                glBindVertexArray(mesh->VAO);
                glDrawElements(GL_TRIANGLES, sm->indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm->indexOffset));
            }
        }
        glBindVertexArray(0); 
    }

    void DrawSubM(SubMesh* sm) {
        glBindVertexArray(mesh->VAO);
        glDrawElements(GL_TRIANGLES, sm->indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm->indexOffset)); 
    }

    //Childs
    void RebuildBatchesWithChilds() {
        batches.clear();
        if (!mesh) return;
        for (auto& sm : mesh->submeshes) {
            Material* mat = sm.material ? sm.material : MaterialDefaults::Get();
            batches[mat].push_back(&sm);
        }
        for (auto* child : mesh->children) if (child) RebuildBatchesChild(child);
    }

    void DrawAll() {
        if (!mesh) return;
        for (auto& [mat, sms] : batches) {
            if (!mat) continue;
            mat->Bind();
            for (auto* sm : sms) {
                glBindVertexArray(mesh->VAO);
                glDrawElements(GL_TRIANGLES, sm->indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm->indexOffset));
            }
        }
        glBindVertexArray(0);
        for (auto* child : mesh->children) if (child) DrawChild(child);
    }

private:
    void RebuildBatchesChild(Mesh* m) {
        for (auto& sm : m->submeshes) {
            Material* mat = sm.material ? sm.material : MaterialDefaults::Get();
            batches[mat].push_back(&sm);
        }
        for (auto* c : m->children) if (c) RebuildBatchesChild(c);
    }

    void DrawChild(Mesh* m) {
        for (auto& sm : m->submeshes) {
            Material* mat = sm.material ? sm.material : MaterialDefaults::Get();
            mat->Bind();
            glBindVertexArray(m->VAO);
            glDrawElements(GL_TRIANGLES, sm.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm.indexOffset));
        }
        for (auto* c : m->children) if (c) DrawChild(c);
    }


//OLD
public:
    // Um material por submesh
    std::vector<Material*> materials; // size == mesh->submeshes.size()

    // Prepara materiais a partir do MeshFilter (copia os defaults da Mesh caso existam)
    void SyncWithMesh(const MeshFilter& filter) {
        materials.clear();
        if (!filter.mesh) return;
        materials.resize(filter.mesh->submeshes.size(), nullptr);
        for (size_t i = 0; i < materials.size(); ++i) {
            auto smMat = filter.mesh->submeshes[i].material;
            materials[i] = smMat ? smMat : MaterialDefaults::Get();
        }
    }

    Material* GetMaterialForSubmesh(size_t i, const MeshFilter& filter) const {
        if (i >= materials.size()) return MaterialDefaults::Get();
        // se null, tenta o da mesh, se null, default
        if (materials[i]) return materials[i];
        if (filter.mesh && i < filter.mesh->submeshes.size() && filter.mesh->submeshes[i].material)
            return filter.mesh->submeshes[i].material;
        return MaterialDefaults::Get();
    }

    void SetMaterialForSubmesh(size_t i, Material* m) {
        if (i >= materials.size()) return;
        materials[i] = m ? m : MaterialDefaults::Get();
    }
};
