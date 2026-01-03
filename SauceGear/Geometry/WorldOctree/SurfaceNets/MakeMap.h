#pragma once 
#include <vector>
#include <queue>
#include <memory>
#include <glm/glm.hpp> 
#include "OctreeLOD.h" 
#include "../Graphics/ComputeShader.h" 

class MakeMap {
public:
    // ---- Octree → Leaf selection ---- 
    void getVoxelLeavesRoot(OctreeNode* n, const glm::vec3& min, const glm::vec3& max, std::vector<OctreeNode*>& out) const { 
        if (!n->getBounds().intersects(min, max)) return;

        if (n->is_leaf()) {
            out.push_back(n);                           //(OctreeNode*)this
            return;
        }
        for (auto& child : n->children) getVoxelLeaves(child, min, max, out);
    }



    void getVoxelLeavesFixedLod(OctreeNode* n, const glm::vec3& min, const glm::vec3& max, std::vector<OctreeNode*>& out, int lod) const { 
        if ( !n->getBounds().iIntersectsU(min, max) || (n->is_leaf() && n->depthLOD > n->targetLOD)) return;
         
        if (n->depthLOD == n->targetLOD) { 
            out.push_back(n);                           //(OctreeNode*)this
            return;
        } 
        for (auto& child : n->children) getVoxelLeavesFixedLod(child, min, max, out, lod);
    }

    void getVoxelLeaves(OctreeNode* n, const glm::vec3& min, const glm::vec3& max, std::vector<OctreeNode*>& out) const {  
        if (!n->getBounds().iIntersectsU(min, max)) return; 

        // First, check if we have already reached the correct octree LOD 
        // (remember that the octree is updated frequently, which may mean that a correct LOD is not necessarily a leaf node), 
        // or check if it is not a leaf node that has not reached a suitable node because it does not have a surface.
        //-----------------condiçao de parada, fallback de seguranca
        if (n->depthLOD == n->targetLOD || (n->is_leaf() && n->depthLOD >= n->targetLOD)) {
            out.push_back(n); 
            return;
        }

        if (n->isChunk())
            for (auto& child : n->children) getVoxelLeavesFixedLod(child, min, max, out, n->targetLOD);
        else
            for (auto& child : n->children) getVoxelLeaves(child, min, max, out);
    } 

    float sample_from_octree(const std::vector<OctreeNode*>& leaves, const glm::vec3& p) {
        for (auto* leaf : leaves)
            if (leaf->contains(p)) return leaf->sdfCenter;    //não faz interpolação trilinear — ele pega diretamente o SDF do nó da octree.

        return 1e6f; // fallback
    } 

    //N = 17 amostras → 16 voxels de espaço cúbico
    glm::vec3 voxel_position(int x, int y, int z, glm::vec3 minCorner, glm::vec3 size) const {
        float C = (float)sysv.get_cellGrid();
        return minCorner + (glm::vec3(x, y, z) / C) * size;                                //float fx = float(x) / resolution; float fy = float(y) / resolution; float fz = float(z) / resolution;
    }

    OctreeNode* find_best_leaf(
        const std::vector<OctreeNode*>& leaves,
        const glm::vec3& p)
    {
        OctreeNode* best = nullptr;
        int bestDepth = -1;

        for (auto* n : leaves) {
            if (!n->getBounds().contains(p)) continue;          //if (!n->contains(p)) continue;

            if (n->depthLOD > bestDepth) {
                bestDepth = n->depthLOD;
                best = n;
            }
        }
        return best;
    }

    SignedDistanceField* sdf = new Terrain();
    float sample_from_octree_cached(
        const std::vector<OctreeNode*>& leaves,
        const glm::vec3& p)
    {
        OctreeNode* n = find_best_leaf(leaves, p);
        if (!n) return sdf->sdfDistance(p);     //      1e6f

        return n->get_value(); // valor filtrado coerente com LOD
    }

    std::vector<float> sample_sdf_grid(OctreeNode* node, const std::vector<OctreeNode*>& leaves) {
        int N = sysv.get_voxelGrid() + sysv.get_Border(); // ex: 17
        std::vector<float> arraySDF(N * N * N); 
        glm::vec3 size = node->b.max - node->b.min;

        for (int z = 0; z < N; ++z)  for (int y = 0; y < N; ++y)  for (int x = 0; x < N; ++x) 
        {
            glm::vec3 p = node->b.min + (glm::vec3(x, y, z) / float(N - 1)) * size;
             
            /*glm::ivec3 p = glm::ivec3(glm::ceil((node->center - node->b.min) / size)) - glm::ivec3(1);
            p = glm::clamp(p, glm::ivec3(0), glm::ivec3(N - 1));*/

            arraySDF[(z * N + y) * N + x] = sample_from_octree_cached(leaves, p);
        }
        return arraySDF;
    }

    std::vector<float> build_logical_grid( OctreeNode& chunk, std::vector<OctreeNode*>& nodes ) {
        int N = sysv.get_voxelGrid();
        std::vector<float> arraySDF(N * N * N); 
        auto idx = [&](int x, int y, int z) { return x + N * (y + N * z); };

        //float nodeSize = chunk.edge_length();
        glm::vec3 nodeSize = chunk.b.max - chunk.b.min;
        AABB bounds = chunk.getBounds(); 

        for (auto* node : nodes) {
            glm::ivec3 pos = glm::ivec3(glm::ceil((node->center - bounds.min) / nodeSize)) - glm::ivec3(1); 
            pos = glm::clamp(pos, glm::ivec3(0), glm::ivec3(N - 1));

            //arraySDF[idx(pos.x, pos.y, pos.z)] = node->get_value();
            arraySDF[idx(pos.x, pos.y, pos.z)] = sdf->sdfDistance(node->center);
        }
        return arraySDF;
    }

    void getSDF_Array(OctreeNode* node) {
        int N = sysv.get_voxelGrid();
        auto idx = [&](int x, int y, int z) { return x + N * (y + N * z); };

        std::vector<float> arraySDF(N * N * N);

    }

    std::vector<float> OcBuilderArraySDF(OctreeNode* node, OctreeNode* root) {
        std::vector<OctreeNode*> leaves;
        //node->b = node->getBounds().expanded(node->edge_length() - 0.001f);
        node->b = node->getBounds();
        root->b = root->getBounds();
        getVoxelLeaves(node, root->b.min, root->b.max, leaves);  //params -> node -- node minCorner - node maxCorner - vector container 
        //getVoxelLeavesFixedLod(node, root->b.min, root->b.max, leaves, node->depthLOD);  //params -> node -- node minCorner - node maxCorner - vector container 
        auto map = sample_sdf_grid(node, leaves);
        
        //auto map = build_logical_grid(*node, leaves);
        return map;
    }

};


/*
void build_logical_grid(
    VoxelOctreeNode& chunk,
    JarVoxelTerrain& terrain,
    const std::vector<VoxelOctreeNode*>& nodes,
    LogicalGrid& grid
) {
    float nodeSize = (1 << chunk.get_lod()) * terrain.get_octree_scale();
    AABB bounds = chunk.get_bounds(terrain.get_octree_scale());

    // limpa
    for (int z = 0; z < CHUNK_RES; z++)  for (int y = 0; y < CHUNK_RES; y++)  for (int x = 0; x < CHUNK_RES; x++)
        grid.nodes[x][y][z] = nullptr;

    for (auto* node : nodes) {
        glm::ivec3 pos = glm::ivec3(glm::ceil((node->_center - bounds.min) / nodeSize)) - glm::ivec3(1);
        pos = glm::clamp(pos, glm::ivec3(0), glm::ivec3(CHUNK_RES - 1));

        grid.nodes[pos.x][pos.y][pos.z] = new ChunkNode{ node->_center, node->get_value() };
    }
}
*/



 /*
OcBuilderArraySDF
for (auto* leaf : leaves) {
    std::cout << " leaf " << node->center.x << " " << node->center.y << " " << node->center.z << std::endl;
    std::cout << " min " << node->b.min.x << " " << node->b.min.y << " " << node->b.min.z << std::endl;
    std::cout << " max " << node->b.max.x << " " << node->b.max.y << " " << node->b.max.z << std::endl;
}
*/