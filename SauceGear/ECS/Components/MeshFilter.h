 #pragma once
#include "../../Resources/Model.h"
 
struct MeshFilter {
    Mesh* mesh = nullptr;

    MeshFilter() = default;
    explicit MeshFilter(Mesh* m) : mesh(m) {}

    void SetFilter(Mesh* newFilter) {
        if (mesh == newFilter)
            return; 
        mesh = newFilter;
    }

    bool HasMesh() const { return mesh != nullptr; }
    Mesh* GetMesh() const { return mesh; }
};
