#pragma once 
#include "../../Resources/Model.h" 

struct MeshFilter {
    Model* model = nullptr;           // Opcional, se veio de um Model
    std::vector<Mesh*> meshes;        // Pode armazenar meshes individuais

    // Construtor para Model
    MeshFilter(Model* m) : model(m) {
        if (model) {
            for (auto& mesh : model->meshes)
                meshes.push_back(&mesh);
        }
    }

    // Construtor para uma única Mesh
    MeshFilter(Mesh* mesh) {
        if (mesh)
            meshes.push_back(mesh);
    }

    // Construtor para várias meshes
    MeshFilter(const std::vector<Mesh*>& meshList) : meshes(meshList) {}

    // Retorna todas as meshes
    std::vector<Mesh*>& GetMeshes() {
        return meshes;
    }

    // Adiciona uma mesh individual
    void AddMesh(Mesh* mesh) {
        if (mesh) meshes.push_back(mesh);
    }


    // Retorna todas as meshes do model
    std::vector<Mesh*> GetMeshesModel() const {
        if (!model) return {};
        std::vector<Mesh*> result;
        for (auto& mesh : model->meshes)
            result.push_back(&mesh);
        return result;
    };

};
