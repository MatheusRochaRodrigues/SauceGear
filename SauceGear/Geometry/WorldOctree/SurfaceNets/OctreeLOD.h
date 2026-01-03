#pragma once
#include <array>
#include <queue>
#include <glm/glm.hpp>
#include <memory>

#include "OctreeDebug.h"
#include "SysVoxel.h" 
#include "SDF_Map.h" 
#include "OctreeNode.h" 

// NVIDIA  = 32 WARP
// AMD     = 64 WAVEFRONT

class OctreeLOD {
public:
    OctreeNode* root = nullptr;
    SDF_Map map;
    std::queue<OctreeNode*> cmptChunkScheduler;

    OctreeLOD(const glm::vec3& center, int lod) {
        root = new OctreeNode{center, lod};            ///new OctreeNode{center, size};
    } 
    ~OctreeLOD() { destroy(root); }  


    inline static int ChildIndex(const OctreeNode* n, const glm::vec3& p) {
        int idx = 0;
        if (p.x >= n->center.x) idx |= 1;
        if (p.y >= n->center.y) idx |= 2;
        if (p.z >= n->center.z) idx |= 4;
        return idx;
    };

    float SampleOctreeSDF(
        const glm::vec3& worldPos,
        int targetChunkLOD
    ) const
    {
        const OctreeNode* node = root;

        while (node->subdivided) {
            if (node->depthLOD <= targetChunkLOD)
                break;

            int ci = ChildIndex(node, worldPos);
            const OctreeNode* child = node->children[ci];

            if (!child || !child->contains(worldPos))
                break;

            node = child;
        }

        // 🔑 SDF linearizado localmente
        return node->sdfCenter
            + glm::dot(node->sdfGradient, worldPos - node->center);
    }



    std::vector<float> BuildChunkSDF(
        OctreeLOD* octree,
        OctreeNode* node
    ) {
        int DIM = sysv.get_voxelGrid() + sysv.get_Border();
        std::vector<float> sdf(DIM * DIM * DIM);

        float voxelSize = node->voxel_size();
        glm::vec3 origin = node->getBounds().min;

        for (int z = 0; z < DIM; ++z)
            for (int y = 0; y < DIM; ++y)
                for (int x = 0; x < DIM; ++x) {

                    glm::vec3 worldPos =
                        origin + (glm::vec3(x, y, z) + 0.5f) * voxelSize;

                    float d = octree->SampleOctreeSDF(worldPos, node->depthLOD);

                    sdf[linearize3(DIM, glm::vec3(x, y, z))] = d;
                }

        return sdf;
    }

    static inline size_t linearize3(size_t cellDimension, glm::vec3 d) {
        return size_t(d.x) + size_t(d.y) * cellDimension + size_t(d.z) * cellDimension * cellDimension;    //x + y * sizeX + z * sizeX * sizeY;
    }


    void Update() {
        std::queue<OctreeNode*> q;          q.push(root); 

        while (!q.empty()) {
            OctreeNode* n = q.front();          q.pop();                                        
            OctreeDebug::PrintNodeHeader(n); 

            //verifica em qual shell está
            n->targetLOD = syso.lod_at(n->center);              // equivalent to the targetLOD 
            if (n->targetLOD < 0) continue;   
             
            if (!n->isEvaluated) {                              //cache interno do octree   
                // --- SDF Distance --- 
                n->sdfCenter = map.sdf->sdfDistance(n->center);

                const float eps = syso.BASE_CELL_SIZE * 0.5f;

                n->sdfGradient = glm::normalize(glm::vec3(
                    map.sdf->sdfDistance(n->center + glm::vec3(eps, 0, 0)) -
                    map.sdf->sdfDistance(n->center - glm::vec3(eps, 0, 0)),

                    map.sdf->sdfDistance(n->center + glm::vec3(0, eps, 0)) -
                    map.sdf->sdfDistance(n->center - glm::vec3(0, eps, 0)),

                    map.sdf->sdfDistance(n->center + glm::vec3(0, 0, eps)) -
                    map.sdf->sdfDistance(n->center - glm::vec3(0, 0, eps))
                ));



                bool hasSurface = map.has_surface(*n, n->sdfCenter, syso.BASE_CELL_SIZE);  
                bool notArrivedLod = (n->depthLOD > n->targetLOD);                             
                                                                    OctreeDebug::PrintSDF(n);                       
                                                                    OctreeDebug::PrintSurfaceDecision(hasSurface);

                //if exists zero-crossing of SDF into of the node  &&  if the current lod of the node have desired Lod based player position
                if ( hasSurface && notArrivedLod && (n->depthLOD > syso.maxDepthLod)) {
                    subdivide(n);                                   OctreeDebug::Subdiveded();   
                    n->isEvaluated = true; 
                }

                // if we don't subdivide further, we mark it as a fully realized subtree
                if (!n->subdivided && ( notArrivedLod || n->depthLOD == syso.maxDepthLod)) {
                    n->isEvaluated = true;
                    materialize(n);                                 OctreeDebug::PrintMaterialize(n);
                    continue;
                }  
            } 

            bool shouldMakeChunk = (n->chunk == nullptr || (n->bounds != computeBounds(n)));
            if ( n->isChunk() && !n->is_leaf() && shouldMakeChunk) {
                QueueChunk(n);                                      OctreeDebug::PrintChunkQueued();
            } 
                
            bool thisNodeIsTrulyChunk = n->isChunk() && (n->chunk != nullptr); 
            //Um nó está materialized quando Ele ou todos os filhos já têm SDF válido, Não precisam mais ser avaliados / subdivididos  -  Ou seja : “Esse pedaço do espaço já está totalmente resolvido.”
            // aqui subdividimos se o algum filho ainda nao foi materializado ou a atual profundidade da octree nao consegue abranger todos os chunks que necessitam de ser construidos
            bool needSubdivide = (!isMaterialized(n) || n->depthLOD > sysv.get_MinChunkLod()); 
            if (n->subdivided && !thisNodeIsTrulyChunk && needSubdivide) {
                for (auto* c : n->children) q.push(c);
            }

            //falta por delete    //if (!is_chunk(terrain)) delete_chunk();  
        }    
        std::cout << std::endl << std::endl << std::endl;           OctreeDebug::PrintTree(root);     std::cout << std::endl;
    }
     
private:  
    void QueueChunk(OctreeNode* n) {
        if (n->isEnqueued) return;
        n->isEnqueued = true;
        cmptChunkScheduler.push(n);
    }

    uint16_t computeBounds(OctreeNode* n) {
        static const glm::vec3 dir[6] = {
            {+1,0,0}, {-1,0,0},
            {0,+1,0}, {0,-1,0},
            {0,0,+1}, {0,0,-1}
        };

        uint16_t b = 0;
        float el = n->edge_length(); // edge_length

        for (int i = 0; i < 6; i++) {
            glm::vec3 p = n->center + dir[i] * el;
            int neighLOD = syso.lod_at(p);

            b |= (n->depthLOD < neighLOD ? 1 : 0) << i;        // high-to-low transition      0 - 6
            b |= (n->depthLOD > neighLOD ? 1 : 0) << (i + 8);  // low-to-high transition      8 - 14
        }
        return b;
    }   
     
    void subdivide(OctreeNode* n) {
        if (n->subdivided) return; 
        
        int   childLOD      = n->depthLOD - 1;                                  //float half     = n->size * 0.5f;
        float childOffset   = n->edge_length() * 0.25f /*(0.5f * 0.5f)*/;    

        for (int i = 0; i < 8; i++) {  
            glm::vec3 offs = {
                (i & 1) ? +childOffset : -childOffset,
                (i & 2) ? +childOffset : -childOffset,
                (i & 4) ? +childOffset : -childOffset
            }; 

            n->children[i] = new OctreeNode{
                n->center + offs,
                childLOD
            }; 
            n->children[i]->father = n;
        } 

        n->subdivided = true;
    }

    inline bool isMaterialized(const OctreeNode* n) { return n->materialized == 0xFF; }

    void materialize(OctreeNode* n) {
        if (isMaterialized(n)) return;
        if (!n->subdivided) n->materialized = 0xFF; 

        else {
            uint8_t mask = 0;
            for (int i = 0; i < 8; i++) 
                if (isMaterialized(n->children[i]))
                    mask |= (1 << i); 

            n->materialized = mask;
        }

        if (isMaterialized(n) && n->father) materialize(n->father);
    }


    bool is_parent_enqueued(OctreeNode* n) { return n->father == nullptr ? false : n->father->isEnqueued; }

    bool is_any_children_enqueued(OctreeNode* n) {
        if (n->is_leaf()) return false;
        for (const auto& child : n->children)  if (child->isEnqueued) return true;
        return false;
    }

    void removeChunk(OctreeNode* n) {
        if (is_any_children_enqueued(n) || is_parent_enqueued(n)) return; 
        //if (n->chunk != nullptr) _chunk->queue_free();  
        //n->chunk = nullptr; 
        n->chunk.reset(); 
    } 

    void destroy(OctreeNode* n) {
        if (!n) return;
        if (n->subdivided) for (auto* c : n->children) destroy(c);
        delete n;
    } 


};




/*  Append
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










/*
                for (auto& child : n->children) child->build(terrain);

void update_node() {
    int desiredLOD = octreeSys->lod_at(n->center);
    n->lod = desiredLOD;
    if (desiredLOD < 0) return;

    if (!n->isAlreadyPass) {  //cache interno do octree 
        // --- SDF Distance ---
        float sdf = map.sdf->sdfDistance(n->center);

        // --- não existe superfície neste nó ---
        if (!map->has_surface(n->center, sdf)) {
            merge(n);
            delete_chunk(n);
            return;
        }

        // --- decidir subdivisão ---
        if (n->size > octreeSys->lod_grid_size(desiredLOD)) {
            // subdividir
            if (!n->subdivided) subdivide(n);
            for (int i = 0; i < 8; i++) update_node(n->children[i]);

            delete_chunk(n);
            return;
        }
    }
    // --- é folha -> deve ter chunk ---
    uint16_t newBound = compute_boundaries(n);
    if (!n->chunk || n->boundaries != newBound) {
        n->boundaries = newBound;
        queue_chunk_update(n);
    }

    mark_materialized(n);
}



void update2() {
    std::queue<OctreeNode*> q;
    q.push(root);

    while (!q.empty()) {
        OctreeNode* n = q.front(); q.pop();

        //verifica em qual shell está
        int targetLOD = octreeSys->lod_at(n->center);

        if (targetLOD < n->lod) {
            if (!n->subdivided)
                subdivide(n);

            for (auto* c : n->children)
                q.push(c);
        }
        else {
            if (n->subdivided)
                merge(n);
        }
    }
}
*/