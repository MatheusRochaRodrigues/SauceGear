#pragma once
#include <array>
#include <queue>
#include <glm/glm.hpp>
#include "SysVoxel.h" 
#include "SDF_Map.h" 
#include "OctreeNode.h" 
#include <memory>
#include "OctreeDebug.h"

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

    void Update() {
        std::queue<OctreeNode*> q;  q.push(root);
        auto& octreeSys = syso.getInstance();

        while (!q.empty()) {
            OctreeNode* n = q.front(); q.pop();

            std::cout << std::endl << std::endl;
            OctreeDebug::PrintNodeHeader(n);

            //verifica em qual shell está
            n->desiredLOD = octreeSys.lod_at(n->center);      // equivalent to the targetLOD 
            if (n->desiredLOD < 0) return;
             
            if (!n->isAlreadyPass) {  //cache interno do octree   
                // --- SDF Distance ---
                float sdf = map.sdf->sdfDistance(n->center);            
                n->distSurf_SDF = sdf;
                bool notArrivedLod = (n->depthLOD > n->desiredLOD);

                OctreeDebug::PrintSDF(n);   
                OctreeDebug::PrintSurfaceDecision(map.has_surface(*n, sdf, octreeSys.BASE_CELL_SIZE));

                //if exists zero-crossing of SDF into of the node  &&  if the current lod of the node have desired Lod based player position
                if ( map.has_surface( *n, sdf, octreeSys.BASE_CELL_SIZE) && notArrivedLod && (n->depthLOD > syso.maxDepthLod) ) {
                    subdivide(n);     OctreeDebug::Subdiveded();  

                    for (auto* c : n->children) q.push(c);
                    n->isAlreadyPass = true; 
                }

                // if we don't subdivide further, we mark it as a fully realized subtree
                //if ( n->is_leaf() && (notArrivedLod || n->depthLOD == octreeSys.maxDepthLod /*0 = default*/) ) {
                //    std::cout << "end "  << std::endl;
                //    n->isAlreadyPass = true; 
                //    //OctreeDebug::PrintMaterialize(n);  
                //    continue;
                //} 
            }
            //std::cout << "+---isChunk() " << n->isChunk() << std::endl; 
            if ( n->isChunk() && !n->is_leaf() && (n->chunk == nullptr || (n->bounds != computeBounds(n))) ) {  
                OctreeDebug::PrintChunkQueued(); 
                QueueChunk(n); 
            } 

            //if ( !n->is_leaf() && (!n->isChunk() && (n->chunk == nullptr))
            //    && ( /*if is above of min lod of chunks*/ n->depthLOD > sysv.get_MinChunkLod()) )  
            //{
            //        for (auto* c : n->children) q.push(c);
            //}


            //falta por delete
            //if (!is_chunk(terrain)) delete_chunk(); 

            /*
            // each chunk is 2^(minChunkLod), ex: 2^4 = 16*16*16 voxels
            uint16_t newBounds = computeBounds(n);
            if (isChunk && !is_leaf() && (n->chunk == nullptr || (n->bounds != newBounds))) {
                n->boundaries = newBound;        //mark_materialized(n);

                queue_update(terrain);  //queue_chunk_update(n);
            }
            */
        } 

        std::cout << std::endl << std::endl << std::endl;
        OctreeDebug::PrintTree(root); 
        std::cout << std::endl;
    }
     
private:
    void evalSDF(OctreeNode& n) {
        Bounds b = n.getBounds();

        n.sdfMin = +FLT_MAX;
        n.sdfMax = -FLT_MAX;

        // 8 cantos do chunk
        for (int i = 0; i < 8; i++) {
            glm::vec3 p = b.corner(i);
            float d = map.sdf->sdfDistance(p);

            n.sdfMin = std::min(n.sdfMin, d);
            n.sdfMax = std::max(n.sdfMax, d);
        }

        n.hasSurface = (n.sdfMin <= 0 && n.sdfMax >= 0);
        n.evaluated = true;
    }

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