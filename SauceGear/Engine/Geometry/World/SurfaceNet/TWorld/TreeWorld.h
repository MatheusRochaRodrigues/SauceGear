#pragma once
#include <array>
#include <queue>
#include <glm/glm.hpp>
#include <memory>

#include "OTreeWorldDebug.h"
#include "SysVoxel.h" 
#include "SDFMap.h" 
#include "OctreeNode.h" 

// NVIDIA  = 32 WARP
// AMD     = 64 WAVEFRONT

class TreeWorld {
public:
    OctreeNode* root = nullptr;
    SDFMap map;
    std::queue<OctreeNode*> cmptChunkScheduler;

    TreeWorld(const glm::vec3& center, int lod) {
        root = new OctreeNode{center, lod};            ///new OctreeNode{center, size};
    } 
    ~TreeWorld() { destroy(root); }  


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

    void Update() {
        std::queue<OctreeNode*> q;          q.push(root); 

        while (!q.empty()) {
            OctreeNode* n = q.front();          q.pop();                                        
            OTreeWorldDebug::PrintNodeHeader(n); 

            //verifica em qual shell está
            n->targetLOD = syso.lod_at(n->center);              // equivalent to the targetLOD 
            if (n->targetLOD < 0) continue;   
             
            if (!n->isEvaluated) {                              //cache interno do octree   
                // --- SDF Distance --- 
                n->sdfCenter = map.sdf->sdfDistance(n->center); 

                bool hasSurface = map.has_surface(*n, n->sdfCenter, syso.BASE_CELL_SIZE);  
                bool notArrivedLod = (n->depthLOD > n->targetLOD);                             
                OTreeWorldDebug::PrintSDF(n);      OTreeWorldDebug::PrintSurfaceDecision(hasSurface);

                //if exists zero-crossing of SDF into of the node  &&  if the current lod of the node have desired Lod based player position
                if ( hasSurface && notArrivedLod && (n->depthLOD > syso.maxDepthLod)) {
                    subdivide(n);                                   OTreeWorldDebug::Subdiveded();   
                    n->isEvaluated = true; 
                }

                // if we don't subdivide further, we mark it as a fully realized subtree
                if (!n->subdivided && ( notArrivedLod || n->depthLOD == syso.maxDepthLod)) {
                    n->isEvaluated = true;
                    materialize(n);                                 OTreeWorldDebug::PrintMaterialize(n);
                    continue;
                }  
            } 

            bool shouldMakeChunk = (n->chunk == nullptr || (n->bounds != computeBounds(n)));
            if ( n->isChunk() && !n->is_leaf() && shouldMakeChunk) {
                QueueChunk(n);                                      OTreeWorldDebug::PrintChunkQueued();
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
        std::cout << std::endl << std::endl << std::endl;           OTreeWorldDebug::PrintTree(root);     std::cout << std::endl;
    }
     
private:   
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

    void QueueChunk(OctreeNode* n) {
        if (n->isEnqueued) return;
        n->isEnqueued = true;
        cmptChunkScheduler.push(n);
    }  

    void destroy(OctreeNode* n) {
        if (!n) return;
        if (n->subdivided) for (auto* c : n->children) destroy(c);
        delete n;
    }
}; 


//


    //bool is_parent_enqueued(OctreeNode* n) { return n->father == nullptr ? false : n->father->isEnqueued; }

    //bool is_any_children_enqueued(OctreeNode* n) {
    //    if (n->is_leaf()) return false;
    //    for (const auto& child : n->children)  if (child->isEnqueued) return true;
    //    return false;
    //} 

    //void removeChunk(OctreeNode* n) {
    //    if (is_any_children_enqueued(n) || is_parent_enqueued(n)) return;
    //    //if (n->chunk != nullptr) _chunk->queue_free();  
    //    //n->chunk = nullptr; 
    //    n->chunk.reset();
    //}