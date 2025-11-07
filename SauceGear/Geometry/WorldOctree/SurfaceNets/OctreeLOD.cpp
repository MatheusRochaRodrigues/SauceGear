#include "OctreeLOD.h"
  

/*


#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <iostream>
#include <cmath>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Graphics/ComputeShader.h"
#include "../Geometry/World/SurfaceNets/SurfaceNetsGPU.h"
#include "../Geometry/World/SurfaceNets/MapGenerator.h"
#include "../Geometry/World/SurfaceNets/GSurfPool.h"

// NOTE: this file is intended to be pasted into your project as a single header/source
// You may need to adjust include paths (glm, sysv, Mesh, Chunk types) to match your project.

// Octree Node
struct OctreeNode {
    glm::vec3 position;   // center
    float size;           // physical size of the cube (edge length)
    int lodLevel = 0;     // 0 = highest detail, larger = coarser
    bool subdivided = false;
    OctreeNode* parent = nullptr;
    OctreeNode* children[8] = { nullptr };

    std::unique_ptr<Chunk> chunk; // contains SDF and mesh

    float dbg = 0.0f; // debug: distance
};

// Settings to tune behaviour in one place
struct LODSettings {
    float baseChunkSize = 25.f;   // physical size of a LOD-0 chunk
    int maxDepth = 4;             // total levels: 0..maxDepth (root usually = maxDepth)
    float shellRadius = 25.f;     // controls how far detail extends (see formula)
    float detailFactor = 1.0f;    // <1 => more detail near camera, >1 => less

    // convenience: compute worldSize for root if needed
    float RootWorldSize() const { return baseChunkSize * std::pow(2.0f, float(maxDepth - 1)); }
};

// Forward: global sysv used in your codebase
#define sysv SysVoxel::getInstance()

// Octree LOD manager
class LODOctree {
public:
    OctreeNode* root = nullptr;
    GPUMapGenerator* generator = nullptr;
    ComputeShader* computeShader = nullptr;

    LODSettings settings;

    LODOctree(GPUMapGenerator* gen, ComputeShader* compute, const glm::vec3& worldCenter, float worldSize )
        : generator(gen), computeShader(compute)
    {
        // ensure settings.maxDepth >= 1
        if (settings.maxDepth < 1) settings.maxDepth = 1;

        // create root node with lod = maxDepth (coarsest)
        root = new OctreeNode{ worldCenter, worldSize, settings.maxDepth, false, nullptr };
    }

    ~LODOctree() {
        DestroyNode(root);
        root = nullptr;
    }

    // Compute LOD level using Chebyshev (inf-norm) and formula in your video.
    // returns value in [0, settings.maxDepth]
    int ComputeLODLevel(const glm::vec3& cameraPos, const glm::vec3& nodeCenter) const
    {
        float dist = glm::compMax(glm::abs(cameraPos - nodeCenter)); // ||p-c||_inf

        // normalized distance relative to shell radius and detailFactor
        float normalized = dist / (2.0f * settings.shellRadius * settings.detailFactor);
        // lod = ceil(log2(max(1, normalized)))
        float lod = std::log2(std::max(1.0f, normalized));
        int level = int(std::ceil(lod));
        if (level < 0) level = 0;
        if (level > settings.maxDepth) level = settings.maxDepth;
        return level;
    }

    // Public API: update tree structure based on camera and generate leaf chunks
    void UpdateAndGenerate(const glm::vec3& cameraPos, bool verbose = false)
    {
        if (!root) return;

        if (verbose) {
            std::cout << "--- LOD Update START (verbose) ---\n";
            std::cout << "Camera: (" << cameraPos.x << "," << cameraPos.y << "," << cameraPos.z << ")\n";
            std::cout << "Settings: shellRadius=" << settings.shellRadius << " detailFactor=" << settings.detailFactor << " maxDepth=" << settings.maxDepth << "\n";
        }

        // Step 1: traverse and decide subdivide/merge based on target LOD
        std::queue<OctreeNode*> q;
        q.push(root);

        while (!q.empty()) {
            OctreeNode* node = q.front(); q.pop();
            if (!node) continue;

            int targetLOD = ComputeLODLevel(cameraPos, node->position);
            node->dbg = glm::compMax(glm::abs(cameraPos - node->position));

            if (verbose) {
                std::cout << "Node center=" << node->position.x << "," << node->position.y << "," << node->position.z
                    << " size=" << node->size << " lod=" << node->lodLevel
                    << " targetLOD=" << targetLOD << " dist_inf=" << node->dbg << "\n";
            }

            // If current level is coarser (larger number) than target, subdivide to go finer
            if (node->lodLevel > targetLOD) {
                if (!node->subdivided && node->lodLevel > 0) {
                    if (verbose) std::cout << "  -> Subdivide\n";
                    Subdivide(node);
                }
            }
            // If current level is finer (smaller number) than target, merge children to go coarser
            else if (node->lodLevel < targetLOD) {
                if (node->subdivided) {
                    if (verbose) std::cout << "  -> Merge\n";
                    Merge(node);
                }
            }

            // push children (if any) to continue traversal
            if (node->subdivided) {
                for (int i = 0; i < 8; ++i) if (node->children[i]) q.push(node->children[i]);
            }
        }

        if (verbose) {
            std::cout << "--- TREE AFTER STRUCTURE UPDATE ---\n";
            PrintOctree(root);
            std::cout << "--- GENERATING LEAF CHUNKS ---\n";
        }

        // Step 2: generate meshes only for leaves
        GenerateLeafChunks(root);

        if (verbose) std::cout << "--- LOD Update END ---\n";
    }

    // collect leaf chunks for rendering
    std::vector<Chunk*> CollectLeafChunks() const
    {
        std::vector<Chunk*> out;
        if (!root) return out;

        std::queue<OctreeNode*> q;
        q.push(root);
        while (!q.empty()) {
            OctreeNode* n = q.front(); q.pop();
            if (!n) continue;
            if (n->subdivided) {
                for (int i = 0; i < 8; ++i) if (n->children[i]) q.push(n->children[i]);
            }
            else {
                if (n->chunk && n->chunk->mesh) out.push_back(n->chunk.get());
            }
        }
        return out;
    }

    // debug print full tree
    void PrintOctree(OctreeNode* node, int depth = 0) const
    {
        if (!node) return;
        std::string indent(depth * 2, ' ');
        std::cout << indent
            << (node->subdivided ? "+" : "-")
            << " LOD " << node->lodLevel
            << " size=" << node->size
            << " pos=(" << node->position.x << "," << node->position.y << "," << node->position.z << ")"
            << " dist_inf=" << node->dbg
            << (node->subdivided ? " (internal)" : " (leaf)")
            << "\n";
        if (node->subdivided) {
            for (int i = 0; i < 8; ++i) PrintOctree(node->children[i], depth + 1);
        }
    }

private:
    void DestroyNode(OctreeNode* node)
    {
        if (!node) return;
        if (node->subdivided) {
            for (int i = 0; i < 8; ++i) DestroyNode(node->children[i]);
        }
        delete node;
    }

    void Subdivide(OctreeNode* node)
    {
        if (!node || node->subdivided || node->lodLevel == 0) return;
        float childSize = node->size * 0.5f;
        int childLOD = node->lodLevel - 1; // finer

        for (int i = 0; i < 8; ++i) {
            // offsets to get centers of children
            glm::vec3 off(
                (i & 1) ? 0.25f * node->size : -0.25f * node->size,
                (i & 2) ? 0.25f * node->size : -0.25f * node->size,
                (i & 4) ? 0.25f * node->size : -0.25f * node->size
            );
            glm::vec3 childCenter = node->position + off;
            node->children[i] = new OctreeNode{ childCenter, childSize, childLOD, false, node };
        }
        node->subdivided = true;
    }

    void Merge(OctreeNode* node)
    {
        if (!node || !node->subdivided) return;
        // delete children
        for (int i = 0; i < 8; ++i) {
            if (node->children[i]) {
                DestroyNode(node->children[i]);
                node->children[i] = nullptr;
            }
        }
        node->subdivided = false;
    }

    void GenerateLeafChunks(OctreeNode* node)
    {
        if (!node) return;
        if (node->subdivided) {
            for (int i = 0; i < 8; ++i) GenerateLeafChunks(node->children[i]);
            return;
        }

        // leaf: ensure chunk exists and generate
        if (!node->chunk) node->chunk = std::make_unique<Chunk>();
        Chunk& chunk = *node->chunk;

        // Determine grid resolution for this node based on its lodLevel.
        // We assume sysv.get_cellGrid() is the cell count for LOD 0 (finest) per axis.
        int baseCells = sysv.get_cellGrid(); // e.g., 32
        int GridCells = std::max(2, baseCells >> node->lodLevel); // coarser with higher lodLevel

        float voxelSize = node->size / float(GridCells); // physical size per voxel

        // resize density map
        size_t voxelCount = size_t(GridCells + 1) * (GridCells + 1) * (GridCells + 1); // points = cells+1
        // but your Chunk expects sysv.get_voxelGrid() sized arrays; adapt if needed
        chunk.resizeDensityMap(); // ensure chunk buffer is large enough for default resolution

        // set offset to the minimum corner of this node
        glm::vec3 offset = node->position - glm::vec3(node->size * 0.5f);

        // Acquire GPU buffer from pool
        SurfaceNetsGPUBuffer* gpuBuf = GlobalSurfaceNetsPool::Get().Acquire();
        gpuBuf->ensureCapacity();

        // Generate SDF into chunk.buff (CPU) using generator (this will upload/download automatically)
        generator->Generate(offset, *chunk.buff, *gpuBuf, GridCells + 1, voxelSize);

        // Build mesh from GPU/CPU SDF
        chunk.mesh = SurfaceNetsGPU::Generate(*chunk.buff, offset, computeShader->ID, *gpuBuf, true, GridCells + 1, voxelSize);

        // release GPU buffer
        GlobalSurfaceNetsPool::Get().Release(gpuBuf);

        // store debug info
        chunk.coord = offset;
        chunk.lod = node->lodLevel;
        chunk.dbg = node->dbg;

        if (chunk.mesh) {
            std::cout << "Generated chunk: center=" << node->position.x << "," << node->position.y << "," << node->position.z
                << " lod=" << node->lodLevel << " GridCells=" << GridCells << " voxelSize=" << voxelSize << "\n";
        }
    }
};



*/