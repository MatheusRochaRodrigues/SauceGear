#pragma once 
#include <vector>   
#include "ConstructMap.h" 
#include "MakeMap.h" 
#include "../ECS/Systems/DebugRenderer.h"

struct ChunkRequest {
    glm::vec3 position;  // centro do chunk
    int lod;             // LOD final
    float grid;          // voxel size (chunkSize << lod)
};

class WorldSys {
public:
    OctreeLOD* octree = nullptr;
    ConstructMap makeMap;

    WorldSys(GPUMapGenerator* gn, ComputeShader* cs) : generator(gn), computeShader(cs){
        octree = new OctreeLOD({ 0,0,0 }, 6 /*6*/);   //2048.0f   //50
    }

    ~WorldSys() { delete octree; }

    void Update(const glm::vec3& p) {
        syso.set_camera(p);
        octree->Update (); 

        // ---- Passo 2: Gera chunks apenas nas folhas ----
        //GenerateLeafChunks(octree->root);
        //return;
        while (!octree->cmptChunkScheduler.empty()) { 
            auto& node = octree->cmptChunkScheduler.front(); 
            octree->cmptChunkScheduler.pop(); 
            UpdateChunk(node);
        }
    }

    inline void DebugPrintSDF(const std::vector<float>& sdf, int dim) {
        int z = dim / 2; // fatia central
        std::cout << "SDF slice at z=" << z << ":\n";

        for (int y = 0; y < dim; ++y) {
            for (int x = 0; x < dim; ++x) {
                float v = sdf[ConstructMap::linearize3(dim, x, y, z)];
                if (v < 0) std::cout << "##";  // dentro da esfera
                else if (v < 1.0f) std::cout << ".."; // superfície
                else std::cout << "  "; // fora
            }
            std::cout << "\n";
        }
    }
     
     
    MakeMap mmap;
    bool UpdateChunk(OctreeNode* n) {  
        int DIM = sysv.get_voxelGrid() + sysv.get_Border();
        float voxelSize = n->voxel_size();                          //n->edge_length() / sysv.get_cellGrid()
        glm::vec3 offset = n->getBounds().min;  //n->getBounds().min - voxelSize;

        //auto map = makeMap.buildDenseSDF(n, DIM, offset);         //auto map = makeMap.buildSDFGrid(n, octree->root); //ck.resizeDensityMap(); // aloca grid denso (ex: 17x17x17)
        auto map = mmap.OcBuilderArraySDF(n, octree->root);         //auto map = makeMap.buildSDFGrid(n, octree->root); //ck.resizeDensityMap(); // aloca grid denso (ex: 17x17x17)
         
        //auto map = octree->BuildChunkSDF(octree, n);


        if (!n->chunk) n->chunk = std::make_unique<Chunk>(sysv.get_voxelGrid() + sysv.get_Border());  
        Chunk& chunk = *n->chunk; chunk.lod = n->depthLOD; chunk.buff->densityMap = map;    // --- cria / redimensiona chunk ---  

        // --- pega buffer GPU ---
        SurfaceNetsGPUBuffer* gpuBuf = GlobalSurfaceNetsPool::Get().Acquire();
        gpuBuf->ensureCapacity(DIM);              //sysv.get_cellGrid() 

        // --- Gera mesh ---
        chunk.mesh = SurfaceNetsGPU::Generate(
            *chunk.buff,
            offset,
            computeShader->ID,
            *gpuBuf,
            false,
            DIM,                //get_cellGrid
            voxelSize       
        );

        GlobalSurfaceNetsPool::Get().Release(gpuBuf);   
        return true;
    }
     

    std::vector<std::pair<Chunk*, OctreeNode*>> CollectChunks() {
        std::vector< std::pair<Chunk*, OctreeNode*> > leafChunks;
        std::queue<OctreeNode*> q;
        q.push(octree->root);

        while (!q.empty()) {
            OctreeNode* node = q.front(); q.pop();

            if (node->subdivided) {
                for (int i = 0; i < 8; i++) q.push(node->children[i]); 
            }

            // nó folha → adiciona se tiver mesh válida
            if (node->chunk && node->chunk->mesh) {
                leafChunks.push_back({ node->chunk.get(), node }); 

                //DebugRenderer::Cube(
                //    node->getBounds().min,
                //    node->getBounds().max,
                //    DebugRenderer::ColorByDepth(node->depthLOD),
                //    true   // Unity-style: redesenha todo frame
                //);
            }
        }

        return leafChunks;
    }

    // Coleta todos os chunks folha (sem gerar nada novo)
    std::vector<Chunk*> CollectLeafChunks() {
        std::vector<Chunk*> leafChunks;
        std::queue<OctreeNode*> q;
        q.push(octree->root);

        while (!q.empty()) {
            OctreeNode* node = q.front(); q.pop();

            if (node->subdivided) {
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
    GPUMapGenerator*    generator = nullptr;
    ComputeShader*      computeShader = nullptr; 
};






//void GenerateChunk(OctreeNode* node)
//{
//    if (!node) return;
//    std::cout << "AQ FOI 2" << std::endl;
//
//    const int grid = sysv.get_cellGrid();           // ex: 16
//    const int lod = node->depthLOD; //lod lodLevel
//
//    // --- voxel size aumenta exponencialmente ---
//    const float voxelSize = sysv.get_baseChunkSize() * float(1 << lod);
//
//    // --- snap-to-grid do GDVoxelTerrain ---
//    const float snapGrid = voxelSize * grid;
//    glm::vec3 offset = glm::floor(node->center / snapGrid) * snapGrid;
//
//    // --- cria / redimensiona chunk ---
//    if (!node->chunk) node->chunk = std::make_unique<Chunk>(grid);
//    Chunk& chunk = *node->chunk;
//
//    // --- pega buffer GPU ---
//    SurfaceNetsGPUBuffer* gpuBuf = GlobalSurfaceNetsPool::Get().Acquire();
//    gpuBuf->ensureCapacity(grid);
//
//    // --- Gera SDF ---
//    generator->Generate(
//        offset,
//        *chunk.buff,
//        *gpuBuf,
//        grid,
//        voxelSize
//    );
//
//    // --- Gera mesh ---
//    chunk.mesh = SurfaceNetsGPU::Generate(
//        *chunk.buff,
//        offset,
//        computeShader->ID,
//        *gpuBuf,
//        true,
//        grid,
//        voxelSize
//    );
//
//    GlobalSurfaceNetsPool::Get().Release(gpuBuf);
//
//    // --- Atualiza estado do chunk ---
//    chunk.coord = offset;
//    chunk.lod = lod;
//}
//
//
//// ---- Collect already-generated chunks ----
//void GenerateLeafChunks(OctreeNode* node) {
//    if (!node) return;
//    if (node->subdivided) { //hasChildren
//        for (int i = 0; i < 8; i++)
//            GenerateLeafChunks(node->children[i]);
//        return;
//    }
//    // Folha -> gerar mesh
//    GenerateChunk(node);
//}