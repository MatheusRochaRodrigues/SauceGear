#pragma once
#include <vector>
#include "LODController.h"
#include "OctreeLOD.h" 

struct ChunkRequest {
    glm::vec3 position;  // centro do chunk
    int lod;             // LOD final
    float grid;          // voxel size (chunkSize << lod)
};

class WorldLODSystem {
public:
    LODController settingsLod;
    OctreeLOD* octree = nullptr;

    WorldLODSystem(GPUMapGenerator* gn, ComputeShader* cs) : generator(gn), computeShader(cs){
        octree = new OctreeLOD({ 0,0,0 }, 50.0f, &settingsLod);   //2048.0f
    }

    ~WorldLODSystem() { delete octree; }

    void Update(const glm::vec3& p) {
        settingsLod.set_camera(p);
        octree->update();

        // ---- Passo 2: Gera chunks apenas nas folhas ----
        GenerateLeafChunks(octree->root);
    }

    void GenerateLeafChunks(OctreeNode* node) {
        if (!node) return; 
        if (node->hasChildren) { //subdivided
            for (int i = 0; i < 8; i++)
                GenerateLeafChunks(node->children[i]);
            return;
        } 
        // Folha -> gerar mesh
        GenerateChunk(node);
    }
    // --------------------------------------------
    // Collect chunk requests for meshing
    // --------------------------------------------
    void collect_chunks(std::vector<ChunkRequest>& out) {
        out.clear();
        collect_recursive(octree->root, out);
    }


    // Coleta todos os chunks folha (sem gerar nada novo)
    std::vector<Chunk*> CollectLeafChunks() {
        std::vector<Chunk*> leafChunks;
        std::queue<OctreeNode*> q;
        q.push(octree->root);

        while (!q.empty()) {
            OctreeNode* node = q.front(); q.pop();

            if (node->hasChildren) {
                for (int i = 0; i < 8; i++)
                    q.push(node->children[i]);
                continue;
            }

            // nó folha → adiciona se tiver mesh válida
            if (node->chunk && node->chunk->mesh) {
                leafChunks.push_back(node->chunk.get());
            }
        }

        return leafChunks;
    }

private:
    GPUMapGenerator* generator = nullptr;
    ComputeShader* computeShader = nullptr;

    void GenerateChunk(OctreeNode* node)
    {
        if (!node) return;

        const int grid = sysv.get_cellGrid();           // ex: 16
        const int lod = node->lod; //lodLevel

        // --- voxel size aumenta exponencialmente ---
        const float voxelSize = settingsLod.baseChunkSize * float(1 << lod);

        // --- snap-to-grid do GDVoxelTerrain ---
        const float snapGrid = voxelSize * grid;
        glm::vec3 offset = glm::floor(node->center / snapGrid) * snapGrid;

        // --- cria / redimensiona chunk ---
        if (!node->chunk)
            node->chunk = std::make_unique<Chunk>(grid);

        Chunk& chunk = *node->chunk;

        // --- pega buffer GPU ---
        SurfaceNetsGPUBuffer* gpuBuf = GlobalSurfaceNetsPool::Get().Acquire();
        gpuBuf->ensureCapacity(grid);

        // --- Gera SDF ---
        generator->Generate(
            offset,
            *chunk.buff,
            *gpuBuf,
            grid,
            voxelSize
        );

        // --- Gera mesh ---
        chunk.mesh = SurfaceNetsGPU::Generate(
            *chunk.buff,
            offset,
            computeShader->ID,
            *gpuBuf,
            true,
            grid,
            voxelSize
        );

        GlobalSurfaceNetsPool::Get().Release(gpuBuf);

        // --- Atualiza estado do chunk ---
        chunk.coord = offset;
        chunk.lod = lod;
    }


    void collect_recursive(OctreeNode* n, std::vector<ChunkRequest>& out) {
        if (!n) return;

        if (!n->hasChildren) {
            float grid = settingsLod.lod_grid_size(n->lod);
            out.push_back({ n->center, n->lod, grid });
            return;
        }

        for (auto* c : n->children)
            collect_recursive(c, out);
    }
};
