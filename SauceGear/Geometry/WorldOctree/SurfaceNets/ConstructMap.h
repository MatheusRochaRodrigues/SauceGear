#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <glm/glm.hpp> 
#include <limits> // Required for std::numeric_limits
#include "OctreeLOD.h"  
  
class ConstructMap {
public:     
    //N = 17 amostras → 16 voxels de espaço cúbico
    glm::vec3 voxel_position(int x, int y, int z, glm::vec3 minCorner, glm::vec3 size) const {
        float resolution = (float)sysv.getInstance().get_cellGrid();
        float fx = float(x) / resolution;
        float fy = float(y) / resolution;
        float fz = float(z) / resolution;

        return minCorner + /*p*/ glm::vec3(fx, fy, fz) * /*size*/ size; //sysv.getInstance().get_baseChunkSize()
    } 

    inline void DebugPrintSDF(const std::vector<float>& sdf, int dim) {
        int z = dim / 2; // fatia central
        std::cout << "SDF slice at z=" << z << ":\n";

        for (int y = 0; y < dim; ++y) {
            for (int x = 0; x < dim; ++x) {
                float v = sdf[linearize3(dim, x, y, z)];
                if (v < 0) std::cout << "##";  // dentro da esfera
                else if (v < 1.0f) std::cout << ".."; // superfície
                else std::cout << "  "; // fora
            }
            std::cout << "\n";
        }
    }
      

    float sampleSDF_LOD(const std::vector<OctreeNode*>& leaves, const glm::vec3& p) {
        for (auto* leaf : leaves)
            if (leaf->contains(p)) return leaf->sdfCenter;    //não faz interpolação trilinear — ele pega diretamente o SDF do nó da octree.

        return std::numeric_limits<float>::max();  //return 1e6f; // fallback
    }

    void getNodesAtDepth(
        OctreeNode* n,
        int targetLOD,
        const AABB& region,
        std::vector<OctreeNode*>& out)
    {
        // Se não intersecta, corta
        if (!n->getBounds().intersects(region.min, region.max)) return;

        // Chegamos no LOD desejado
        if (n->depthLOD == targetLOD) {
            out.push_back(n);
            return;
        }

        // Passamos do LOD e este nó nunca subdividiu
        if (!n->subdivided && n->depthLOD > targetLOD) {
            out.push_back(n);
            return;
        }

        // Senão continua descendo
        for (auto* c : n->children) getNodesAtDepth(c, targetLOD, region, out);
    } 

    std::vector<float> buildSDFGrid(OctreeNode* chunkNode, OctreeNode* root)
    {
        int N = 17; // resolução do sample
        std::vector<float> sdf(N * N * N);

        AABB region = chunkNode->getBounds();
        int targetLOD = chunkNode->depthLOD;

        std::vector<OctreeNode*> nodes;
        getNodesAtDepth(root, targetLOD, region, nodes);

        glm::vec3 size = region.max - region.min;

        for (int z = 0; z < N; ++z)
            for (int y = 0; y < N; ++y)
                for (int x = 0; x < N; ++x)
                {
                    glm::vec3 p = region.min + glm::vec3(
                        (float)x / 16.0f,
                        (float)y / 16.0f,
                        (float)z / 16.0f
                    ) * size;

                    sdf[(z * N + y) * N + x] = sampleSDF_LOD(nodes, p);
                }

        return sdf;
    }
     
    SDF_Map map;
    std::vector<float> buildDenseSDF(OctreeNode* chunk) { 
        int cells = sysv.get_cellGrid();                                // 16
        int N = cells + 1 + sysv.get_Border()/*1 if true else 0*/;      // 16 (cells) + 1(to voxels) + 1 (border) 

        std::vector<float> grid(N * N * N);

        AABB region = chunk->getBounds();
        glm::vec3 size = region.max - region.min;
        glm::vec3 minCorner = chunk->center - glm::vec3(size * 0.5f);

        float voxelSize = chunk->edge_length() / float(cells);

        // ⚠️ desloca 1 voxel PARA FORA 
        glm::vec3 origin;
        if(sysv.get_Border() == 2) origin = minCorner - voxelSize;
        else origin = minCorner;    //default

        for (int z = 0; z < N; z++)
            for (int y = 0; y < N; y++)
                for (int x = 0; x < N; x++) { 
                    glm::vec3 p = origin + glm::vec3(x, y, z) * voxelSize;
                    grid[(z * N + y) * N + x] = map.sdf->sdfDistance(p);
                }

        return grid;
    }
     

    // Indexação 3D → 1D (linear) // helper linearize function - assumes row-major x + y*nx + z*nx*ny
    static inline size_t linearize3(size_t dim, uint32_t x, uint32_t y, uint32_t z) { return x + y * dim + z * dim * dim; }
    static inline size_t linearize3(size_t cellDimension, glm::vec3 d) {
        return size_t(d.x) + size_t(d.y) * cellDimension + size_t(d.z) * cellDimension * cellDimension;    //x + y * sizeX + z * sizeX * sizeY;
    }
};


 