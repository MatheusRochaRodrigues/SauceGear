#pragma once
#include <unordered_map>
#include <vector>
#include "../../Resources/DefineMaterials/MaterialInstance.h"
#include "../../ECS/Components/MeshFilter.h"
#include "../../Resources/DataBase/AssetDatabase.h"

struct MeshRenderer {
    Mesh* mesh = nullptr; 
    // WARNING ---- ISSO ASSUME Q É O UNICO DETENTOR DE MESH, LOGO SE O CONTEXTO FOR TROCARDO, MESH RENDERER SER DELETADO PELO SHARED_PTR
    //std::shared_ptr<Mesh> mesh; // referência ao mesh da entidade 
     
    REFLECT_CLASS(MeshRenderer) {
        REFLECT_HEADER("MeshRenderer");
        REFLECT_FIELD(mesh);
    }

    std::unordered_map<std::shared_ptr<MaterialInstance>, std::vector<SubMesh*>> batches; 
      
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
            if (!sm.material) {
                std::cout << "[DEBUG] - material gerado em RebuildBatches" << std::endl; 
                //sm.material = (sm.material) ? sm.material : AssetDatabase::Load<MaterialInstance>("__default_material__");   //sm.material = std::make_shared<MaterialInstance>(std::make_shared<PBRMaterial>());
                static auto defaultMat = AssetDatabase::Load<MaterialInstance>("__default_material__", [] {
                    return std::make_shared<MaterialInstance>(std::make_shared<PBRMaterial>());
                });
                sm.material = defaultMat;
            } 
            batches[sm.material].push_back(&sm);
        }
    }

    /*void Render(Shader* overrideShader = nullptr) {
        if (!mesh) return;
        for (size_t i = 0; i < mesh->GetSubMeshes().size(); ++i) {
            auto& sub = mesh->GetSubMeshes()[i];
            if (i < materials.size() && materials[i]) {
                materials[i]->Apply(overrideShader);
            }
            mesh->DrawSubMesh(i);
        }
    }*/

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

    /*void Draw(std::shared_ptr<MaterialInstance> mat) {
        if (!mesh) return;
        if (!mat) return;
        for (auto& sm : batches[mat]) { 
            glBindVertexArray(mesh->VAO);
            glDrawElements(GL_TRIANGLES, sm->indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm->indexOffset)); 
        }
        glBindVertexArray(0);
    }*/

    void Paint() {
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
            auto mat = sm.material; //Material* mat = sm.material ? sm.material : MaterialDefaults::Get();
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
            auto mat = sm.material; //Material* mat = sm.material ? sm.material : MaterialDefaults::Get();
            batches[mat].push_back(&sm);
        }
        for (auto* c : m->children) if (c) RebuildBatchesChild(c);
    }

    void DrawChild(Mesh* m) {
        for (auto& sm : m->submeshes) {
            auto mat = sm.material; //Material* mat = sm.material ? sm.material : MaterialDefaults::Get();
            mat->Bind();
            glBindVertexArray(m->VAO);
            glDrawElements(GL_TRIANGLES, sm.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * sm.indexOffset));
        }
        for (auto* c : m->children) if (c) DrawChild(c);
    }


//OLD
public:
    /*
    // Um material por submesh
    std::vector<std::shared_ptr<MaterialInstance>> materials; 

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
    */
};
