#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <queue>
#include "GPUMapGenerator.h"
#include "SurfaceNetsGPU.h"

struct OctreeNode {
    glm::vec3 position;      // centro do chunk
    float size;              // tamanho do chunk
    int lodLevel;            // LOD atual
    bool subdivided = false;
    OctreeNode* parent = nullptr;
    OctreeNode* children[8] = { nullptr };

    ChunkBuffer chunkCPU;
    SurfaceNetsGPUBuffer chunkGPU;
};

class LODOctree {
public:
    OctreeNode* root;
    GPUMapGenerator* generator;
    int maxLOD = 3;
    float lodDistance[4] = { 100.0f, 50.0f, 25.0f, 12.5f }; // distâncias limites por LOD

    LODOctree(GPUMapGenerator* gen, glm::vec3 worldCenter, float worldSize) {
        generator = gen;
        root = new OctreeNode{ worldCenter, worldSize, 0 };
    }

    void UpdateLOD(const glm::vec3& camPos) {
        std::queue<OctreeNode*> q;
        q.push(root);

        while (!q.empty()) {
            OctreeNode* node = q.front(); q.pop();
            float dist = glm::distance(camPos, node->position);

            // Decide se subdivide ou merge
            if (dist < lodDistance[node->lodLevel] && node->lodLevel < maxLOD) {
                if (!node->subdivided) Subdivide(node);
                for (int i = 0; i < 8; i++) q.push(node->children[i]);
            }
            else {
                if (node->subdivided) Merge(node);
            }

            // Atualiza GPU chunk
            generator->Generate(node->position - glm::vec3(node->size / 2.0f),
                node->chunkCPU, node->chunkGPU);

            // Stitch bordas com vizinhos 
        }
    }

private:
    void Subdivide(OctreeNode* node) {
        float hs = node->size * 0.5f;
        int lod = node->lodLevel + 1;
        for (int i = 0; i < 8; i++) {
            glm::vec3 offset(
                (i & 1 ? 0.25f : -0.25f) * node->size,
                (i & 2 ? 0.25f : -0.25f) * node->size,
                (i & 4 ? 0.25f : -0.25f) * node->size
            );
            node->children[i] = new OctreeNode{ node->position + offset, hs, lod, false, node };
        }
        node->subdivided = true;
    }

    void Merge(OctreeNode* node) {
        if (!node->subdivided) return;
        for (int i = 0; i < 8; i++) {
            delete node->children[i];
            node->children[i] = nullptr;
        }
        node->subdivided = false;
    }

    // ================= Stitching =================== 
};




/*
for (int i = 0; i < 8; i++) 
    glm::vec3 offset(
        (i & 1 ? 0.25f : -0.25f) * node->size,
        (i & 2 ? 0.25f : -0.25f) * node->size,
        (i & 4 ? 0.25f : -0.25f) * node->size
    );

Child 0 (i=0, bits=000): (-0.5, -0.5, -0.5)
Child 1 (i=1, bits=001): (+0.5, -0.5, -0.5)
Child 2 (i=2, bits=010): (-0.5, +0.5, -0.5)
Child 3 (i=3, bits=011): (+0.5, +0.5, -0.5)
Child 4 (i=4, bits=100): (-0.5, -0.5, +0.5)
Child 5 (i=5, bits=101): (+0.5, -0.5, +0.5)
Child 6 (i=6, bits=110): (-0.5, +0.5, +0.5)
Child 7 (i=7, bits=111): (+0.5, +0.5, +0.5)

*/