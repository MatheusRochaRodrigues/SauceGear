#pragma once
#include <iostream>
#include "../Assets/MeshAsset.h"

struct RenderItem {
    MeshAsset* mesh;
    uint32_t          submesh;
    MaterialInstance* material;
    glm::mat4         model;
};

//for Batching
struct BatchKey {
    Shader* shader;
    MeshAsset* mesh;
    uint32_t   submesh;
    MaterialAsset* material;

    bool operator==(const BatchKey& o) const {
        return shader == o.shader &&
            mesh == o.mesh &&
            submesh == o.submesh &&
            material == o.material;
    }
};

struct BatchKeyHash {
    size_t operator()(const BatchKey& k) const {
        size_t h = 0;
        h ^= std::hash<void*>{}(k.shader) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<void*>{}(k.mesh) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint32_t>{}(k.submesh);
        h ^= std::hash<void*>{}(k.material) + 0x9e3779b9;
        return h;
    }
};

struct RenderBatch {
    BatchKey key;
    std::vector<glm::mat4> instances;
};








/*  ORDEM DE RENDERIZAÇÃO

Scene / ECS / VoxelSystem
        │
        ▼
 RenderQueue::Submit()
        │
        ▼
 RenderQueue::BuildBatches()
        │
        ▼
 GeometryPass::Execute()
        │
        ▼
  Draw (instanced ou normal)


*/