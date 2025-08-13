
/*
#pragma once 
//#include "../../Core/EngineContext.h"
#include "../../Resources/Model.h"
#include "../../Scene/Components/Material.h"
#include "../../Scene/Components/MeshFilter.h"

struct MeshRenderer {
    MeshFilter* filter = nullptr;  // Referência para a geometria
    // Chave = ponteiro de material, Valor = vetor de meshes usando esse material
    std::unordered_map<Material*, std::vector<Mesh*>> batches;

    MeshRenderer() = default;

    MeshRenderer(MeshFilter* f = nullptr) : filter(f) {
        if (filter) RebuildBatches();
    }

    void SetFilter(MeshFilter* f) {
        filter = f;
        RebuildBatches();
    }

    // Adiciona mesh com material (ou default)
    void Add(Mesh* mesh, Material* material = nullptr) {
        if (!mesh) return;
        Material* mat = material ? material : MaterialDefaults::Get();
        batches[mat].push_back(mesh);
    }

    // Troca material de uma mesh
    void SetMaterial(Mesh* mesh, Material* newMaterial) {
        if (!mesh) return;
        Material* mat = newMaterial ? newMaterial : MaterialDefaults::Get();

        // Remove mesh de batches anteriores
        for (auto& [m, meshes] : batches) {
            auto it = std::find(meshes.begin(), meshes.end(), mesh);
            if (it != meshes.end()) {
                meshes.erase(it);
                break;
            }
        }

        batches[mat].push_back(mesh);
    }
     

    void RebuildBatches() {
        batches.clear();
        if (!filter) return;

        for (auto* mesh : filter->GetMeshes()) {
            Material* mat = filter->GetMaterialForMesh(mesh); // pega material do Model
            //if (mat == nullptr) mat = MaterialDefaults::Default();
            batches[mat].push_back(mesh);
        }
    }

    // Reconstrói os batches usando o MeshFilter
    //void RebuildBatches() {
    //    batches.clear();
    //    if (!filter) return;

    //    for (auto* mesh : filter->GetMeshes()) {
    //        // Cria material automaticamente se não existir
    //        Material* mat = new Material();
    //        mat->floatParams["roughness"] = 0.5f;
    //        mat->floatParams["metallic"] = 0.1f;
    //        Add(mesh, mat);
    //    }
    //}

    // Renderiza todas as meshes agrupadas por material
    void DrawAll() const {
        for (auto& [material, meshes] : batches) {
            if (!material) continue;
            material->Bind();

            // Aqui você poderia implementar instancing se meshes compartilharem a mesma geometria
            for (auto* mesh : meshes) {
                if (mesh) mesh->Draw();
            }
        }
    }

    // Renderização instanciada
    void DrawInstanced(Material* material, GLsizei instanceCount) const {
        auto it = batches.find(material);
        if (it == batches.end()) return;

        material->Bind();
        for (auto* mesh : it->second) {
            if (!mesh) continue;
            mesh->DrawInstanced(instanceCount);
        }
    }

    // Desenha instanciado
    void DrawInstanced(GLsizei instanceCount) const {
        for (auto& [material, meshes] : batches) {
            if (!material) continue;
            material->Bind();
            for (auto* mesh : meshes) {
                if (mesh) mesh->DrawInstanced(instanceCount);  // Mesma geometria, multiplas instâncias
            }
        }
    }
};

 */
 



#pragma once

//#include "../../Core/EngineContext.h"
#include "../../Resources/Model.h"
#include "../../Scene/Components/Material.h"

struct MeshRenderer {
    //Material* material = nullptr;

    Model* model = nullptr;

    // Um material por mesh (índice corresponde ao model->meshes)
    //std::vector<Material*> materials;

    MeshRenderer(Model* m = nullptr) : model(m) {}      //, const std::vector<Material*>& mats = {}          materials(mats) 


    Material* GetMaterial(size_t meshIndex) {
        if (!model || meshIndex >= model->meshes.size()) {
            std::cout << "material fora do alcance da mesh";
            return MaterialDefaults::Get();
        }
        auto& mesh = model->meshes[meshIndex];
        if (!mesh.material) mesh.material = MaterialDefaults::Get();
        return mesh.material;
    }

    void SetMaterial(size_t meshIndex, Material* mat) {
        if (!model || meshIndex >= model->meshes.size()) {
            std::cout << "you try set material that not exists";
            return;
        };
        model->meshes[meshIndex].material = mat;
    }

};

