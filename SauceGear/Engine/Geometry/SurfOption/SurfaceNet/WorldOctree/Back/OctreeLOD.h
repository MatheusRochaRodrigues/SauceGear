#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <iostream>
#include "../Graphics/ComputeShader.h" 
#include "../Geometry/World/SurfaceNets/SurfaceNetsGPU.h"   
#include "../Geometry/World/SurfaceNets/MapGenerator.h"    
#include "../Geometry/World/SurfaceNets/GSurfPool.h"    

// Octree Node para chunks
struct OctreeNode {
    glm::vec3 position;
    float size;
    int lodLevel = 0;
    bool subdivided = false;
    OctreeNode* parent = nullptr;
    OctreeNode* children[8] = { nullptr };

    std::unique_ptr<Chunk> chunk;
};

// Octree LOD
class LODOctree {
public:
    OctreeNode* root;
    GPUMapGenerator* generator;
    int maxLOD = 3;
    float lodDistance[4] = { 100.f, 50.f, 25.f, 12.5f };

    LODOctree(GPUMapGenerator* gen, glm::vec3 worldCenter, float worldSize) {
        generator = gen;
        root = new OctreeNode{ worldCenter, worldSize, 0 };
    }

    // Atualiza LOD e gera chunks
    void UpdateLOD(const glm::vec3& camPos) {
        std::queue<OctreeNode*> q;
        q.push(root);

        while (!q.empty()) {
            OctreeNode* node = q.front(); q.pop();
            float dist = glm::distance(camPos, node->position);

            if (dist < lodDistance[node->lodLevel] && node->lodLevel < maxLOD) {
                if (!node->subdivided) Subdivide(node);
                for (int i = 0; i < 8; i++) q.push(node->children[i]);
            }
            else {
                if (node->subdivided) Merge(node);
            }

            // Se ainda não tiver chunk, cria
            if (!node->chunk) {
                node->chunk = std::make_unique<Chunk>();
            }

            // Offset do chunk baseado no centro
            glm::vec3 offset = node->position - glm::vec3(node->size / 2.0f);

            auto& vBuff = *node->chunk->buff.get();

            // Acquire GPU buffer
            SurfaceNetsGPUBuffer* gpuBuf = GlobalSurfaceNetsPool::Get().Acquire();
            gpuBuf->ensureCapacity();

            // Gera SDF no GPU e mantém no CPU
            generator->Generate(offset, vBuff, *gpuBuf);

            // Gera mesh
            node->chunk->mesh = SurfaceNetsGPU::Generate(vBuff, offset, generator->compute.ID, *gpuBuf, true);

            // Release GPU buffer (dados já copiados para CPU)
            GlobalSurfaceNetsPool::Get().Release(gpuBuf);
        }
    }

private:
    void Subdivide(OctreeNode* node) {
        float hs = node->size * 0.5f;
        int lod = node->lodLevel + 1;
        for (int i = 0; i < 8; i++) {
            glm::vec3 offset(
                (i & 1 ? -0.25f : 0.25f) * node->size,
                (i & 2 ? -0.25f : 0.25f) * node->size,
                (i & 4 ? -0.25f : 0.25f) * node->size
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
};
