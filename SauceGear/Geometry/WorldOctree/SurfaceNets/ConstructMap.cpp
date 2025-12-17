//#pragma once
//#include <vector>
//#include <queue>
//#include <memory>
//#include <glm/glm.hpp> 
//#include "OctreeLOD.h" 
//#include "../Graphics/ComputeShader.h" 
//
//class ConstructMap {
//public:
//    // ---- Octree → Leaf selection ---- 
//    void getVoxelLeavesRoot(OctreeNode* n, const glm::vec3& min, const glm::vec3& max, std::vector<OctreeNode*>& out) const {
//        //if (! n->b.intersects(min, max)) return;
//        if (!n->getBounds(syso.octreeScale).intersects(min, max)) return;
//
//        if (n->is_leaf()) {
//            out.push_back(n);        //(OctreeNode*)this
//            return;
//        }
//        for (auto& child : n->children) getVoxelLeaves(child, min, max, out);
//    }
//
//    void getVoxelLeavesFixedLod(OctreeNode* n, const glm::vec3& min, const glm::vec3& max, std::vector<OctreeNode*>& out, int lod) const {
//
//        std::cout << "6" << std::endl;
//        //if (!n->getBounds(syso.octreeScale).intersects(min, max)
//        if (!n->getBounds(syso.octreeScale).intersects(min, max, n->b.min, n->b.max)
//            || (n->is_leaf() && n->depthLOD > n->desiredLOD)) return;
//
//        std::cout << "7" << std::endl;
//        if (n->depthLOD == n->desiredLOD) {
//            std::cout << "8" << std::endl;
//            out.push_back(n);        //(OctreeNode*)this
//            return;
//        }
//
//        for (auto& child : n->children) getVoxelLeavesFixedLod(child, min, max, out, lod);
//    }
//
//    void getVoxelLeaves(OctreeNode* n, const glm::vec3& min, const glm::vec3& max, std::vector<OctreeNode*>& out) const {
//        std::cout << "3" << std::endl;
//        //if (!n->getBounds(syso.octreeScale).intersects(min, max)) return;
//        if (!n->getBounds(syso.octreeScale).intersects(min, max, n->b.min, n->b.max)) return;
//
//        std::cout << "4" << std::endl;
//
//        //First, check if we have already reached the correct octree LOD 
//        // (remember that the octree is updated frequently, which may mean that a correct LOD is not necessarily a leaf node), 
//        // or check if it is not a leaf node that has not reached a suitable node because it does not have a surface.
//        if (n->depthLOD == n->desiredLOD || (n->is_leaf() && n->depthLOD >= n->desiredLOD)) {
//            out.push_back(n);
//            std::cout << "5" << std::endl;
//            return;
//        }
//
//        if (n->isChunk())
//            for (auto& child : n->children) getVoxelLeavesFixedLod(child, min, max, out, n->desiredLOD);
//        else
//            for (auto& child : n->children) getVoxelLeaves(child, min, max, out);
//    }
//
//    //N = 17 amostras → 16 voxels de espaço cúbico
//    glm::vec3 voxel_position(int x, int y, int z, glm::vec3 minCorner, glm::vec3 size) const {
//        float resolution = (float)sysv.getInstance().get_cellGrid();
//        float fx = float(x) / resolution;
//        float fy = float(y) / resolution;
//        float fz = float(z) / resolution;
//
//        return minCorner + /*p*/ glm::vec3(fx, fy, fz) * /*size*/ size; //sysv.getInstance().get_baseChunkSize()
//    }
//
//    float sample_from_octree(const std::vector<OctreeNode*>& leaves, const glm::vec3& p) {
//        for (auto* leaf : leaves)
//            if (leaf->contains(p)) return leaf->distSurf_SDF;    //não faz interpolação trilinear — ele pega diretamente o SDF do nó da octree.
//
//        return 1e6f; // fallback
//    }
//
//    inline void DebugPrintSDF(const std::vector<float>& sdf, int dim) {
//        int z = dim / 2; // fatia central
//        std::cout << "SDF slice at z=" << z << ":\n";
//
//        for (int y = 0; y < dim; ++y) {
//            for (int x = 0; x < dim; ++x) {
//                float v = sdf[GeneratorMap::linearize3(dim, x, y, z)];
//                if (v < 0) std::cout << "##";  // dentro da esfera
//                else if (v < 1.0f) std::cout << ".."; // superfície
//                else std::cout << "  "; // fora
//            }
//            std::cout << "\n";
//        }
//    }
//
//    std::vector<float> sample_sdf_grid(glm::vec3 minCorner, glm::vec3 maxCorner, const std::vector<OctreeNode*>& leaves) {
//        int N = sysv.getInstance().get_voxelGrid(); // 17 - resolution
//        std::vector<float> arraySDF(N * N * N);
//
//        glm::vec3 size = maxCorner - minCorner;
//
//        std::cout << " 1 " << std::endl;
//        for (auto* leaf : leaves) {
//            std::cout << " 2 " << std::endl;
//            std::cout
//                << "Leaf bounds: "
//                << leaf->b.min.x << ", " << leaf->b.min.y << ", " << leaf->b.min.z
//                << " -> "
//                << leaf->b.max.x << ", " << leaf->b.max.y << ", " << leaf->b.max.z
//                << std::endl;
//        }
//
//
//
//        for (int z = 0; z < N; ++z)
//            for (int y = 0; y < N; ++y)
//                for (int x = 0; x < N; ++x)
//                {
//                    glm::vec3 p = voxel_position(x, y, z, minCorner, size);
//
//                    //std::cout << "voxel p=" << p.x << "," << p.y << "," << p.z << std::endl;
//
//                    float d = sample_from_octree(leaves, p);
//
//                    int indice = (z * N + y) * N + x;
//                    arraySDF[indice] = d;
//                    //std::cout << " fd ç " << arraySDF[indice] << std::endl;
//                }
//
//
//        //DebugPrintSDF(arraySDF, sysv.get_voxelGrid());
//        return arraySDF;
//    }
//
//    std::vector<float> BuilderArraySDF(OctreeNode* node, OctreeNode* root) {
//        std::vector<OctreeNode*> leaves;
//        node->b = node->getBounds();
//        getVoxelLeaves(node, root->b.min, root->b.max, leaves);  //params -> node -- node minCorner - node maxCorner - vector container
//
//
//        for (auto* leaf : leaves) {
//            std::cout << " leaf " << node->center.x << " " << node->center.y << " " << node->center.z << std::endl;
//            std::cout << " min " << node->b.min.x << " " << node->b.min.y << " " << node->b.min.z << std::endl;
//            std::cout << " max " << node->b.max.x << " " << node->b.max.y << " " << node->b.max.z << std::endl;
//        }
//
//        auto map = sample_sdf_grid(node->b.min, node->b.max, leaves);
//
//        return map;
//    }
//
//};




/*


//class OctreeNode;
//class Chunk;
//class SurfaceNetsGPUBuffer;
//class SurfaceNetsGPU;
//class GlobalSurfaceNetsPool;
//class ComputeShader;
//
//struct SysVoxel;
//struct SysOctree;




// ---- Octree → Leaf selection ----
void getVoxelLeaves(OctreeNode* n,
    const glm::vec3& min,
    const glm::vec3& max,
    std::vector<OctreeNode*>& out) const;

void getVoxelLeavesRoot(OctreeNode* n,
    const glm::vec3& min,
    const glm::vec3& max,
    std::vector<OctreeNode*>& out) const;

void getVoxelLeavesFixedLod(OctreeNode* n,
    const glm::vec3& min,
    const glm::vec3& max,
    std::vector<OctreeNode*>& out,
    int lod) const;

// ---- SDF sampling ----
glm::vec3 voxel_position(int x, int y, int z,
    glm::vec3 minCorner) const;

float sample_from_octree(const std::vector<OctreeNode*>& leaves,
    const glm::vec3& p);

std::vector<float> sample_sdf_grid(glm::vec3 minCorner,
    const std::vector<OctreeNode*>& leaves);

std::vector<float> BuilderArraySDF(OctreeNode* node);

*/