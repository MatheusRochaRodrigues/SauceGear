#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <glm/glm.hpp> 
#include "OctreeLOD.h" 
#include "../Graphics/ComputeShader.h" 
#include <limits> // Required for std::numeric_limits
  
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
                float v = sdf[GeneratorMap::linearize3(dim, x, y, z)];
                if (v < 0) std::cout << "##";  // dentro da esfera
                else if (v < 1.0f) std::cout << ".."; // superfície
                else std::cout << "  "; // fora
            }
            std::cout << "\n";
        }
    }
      

    float sampleSDF_LOD(const std::vector<OctreeNode*>& leaves, const glm::vec3& p) {
        for (auto* leaf : leaves)
            if (leaf->contains(p)) return leaf->distSurf_SDF;    //não faz interpolação trilinear — ele pega diretamente o SDF do nó da octree.

        return std::numeric_limits<float>::max();  //return 1e6f; // fallback
    }

    void getNodesAtDepth(
        OctreeNode* n,
        int targetLOD,
        const Bounds& region,
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

        Bounds region = chunkNode->getBounds();
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


    std::vector<float> buildDenseSDF(OctreeNode* chunk) {
        SDF_Map map;

        int N = 17; // 17 por exemplo
        std::vector<float> grid(N * N * N);

        Bounds region = chunk->getBounds();
        glm::vec3 size = region.max - region.min;
        glm::vec3 minCorner = chunk->center - glm::vec3(size * 0.5f);

        for (int z = 0; z < N; z++)
            for (int y = 0; y < N; y++)
                for (int x = 0; x < N; x++) {
                    glm::vec3 p = minCorner + size * glm::vec3(
                        float(x) / float(N - 1),
                        float(y) / float(N - 1),
                        float(z) / float(N - 1)
                    );

                    grid[(z * N + y) * N + x] = map.sdf->sdfDistance(p);
                }

        return grid;
    }

     
};


 