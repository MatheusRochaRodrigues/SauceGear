#pragma once
#include <array>
#include <queue>
#include <glm/glm.hpp>
#include "OctreeSys.h" 
#include "Map.h" 

struct OctreeNode {
    glm::vec3 center;
    int depthLOD;       //  0 for the most detailed chuck
    //float size;

    bool subdivided = false;
    std::array<OctreeNode*, 8> children = { nullptr };
    OctreeNode* father = nullptr;
    float distSurf_SDF;
    //Mesh
    std::unique_ptr<Chunk> chunk;

    // ---------- cache Otimization ----------
    bool isAlreadyPass = false;         // já processado (como no GDVoxel)
    bool isEnqueued = false;   // se já está enfileirado para gerar mesh
    uint16_t bounds = 0;      // 12 bits (6 highs / 6 lows)
    uint8_t materialized = 0;   // 8 bits (cada child materialized)
     
    inline float edge_length(float scale) const { return (1 << depthLOD) * scale; } //_size
};

class OctreeLOD {
public:
    OctreeNode* root = nullptr;
    OctreeSys* octreeSys; 
    Map map;

    OctreeLOD(const glm::vec3& center, float size, OctreeSys* l) : octreeSys(l) {
        root = new OctreeNode{ center, size, l->lodCount - 1 };
    } 
    ~OctreeLOD() { destroy(root); }  

    void Update() {
        std::queue<OctreeNode*> q;  q.push(root);

        while (!q.empty()) {
            OctreeNode* n = q.front(); q.pop();
            //verifica em qual shell está
            int desiredLOD = octreeSys->lod_at(n->center);      // equivalent to the targetLOD 
            if (desiredLOD < 0) return;

            if (!n->isAlreadyPass) {  //cache interno do octree   
                // --- SDF Distance ---
                float sdf = map.sdf->sdfDistance(n->center);            n->distSurf_SDF = sdf;
                bool notArrivedLod = (n->depthLOD > desiredLOD);

                //if exists zero-crossing of SDF into of the node  &&  if the current lod of the node have desired Lod based player position
                if ( map.has_surface(*n, sdf, octreeSys->octreeScale) && notArrivedLod ) {
                    if (!n->subdivided) subdivide(n); 
                    //for (auto* c : n->children) q.push(c);
                    n->isAlreadyPass = true;
                    continue;
                }

                // if we don't subdivide further, we mark it as a fully realized subtree
                if ( is_leaf(n) && (notArrivedLod || n->depthLOD == octreeSys->maxDepthLod /*0 = default*/) ) {
                    n->isAlreadyPass = true;
                    materialize(n);
                    continue;
                } 
            } 
            bool isChunk = (n->depthLOD == (desiredLOD + octreeSys->minChunkLod));

            if (isChunk && !is_leaf(n) && (n->chunk == nullptr || (n->bounds != computeBounds(n)))) QueueChunk(n);
             
            //talvez
            // sempre processar nós da octree enquanto houver filhos
            /*if (!is_leaf()) {
                for (auto& child : *_children)
                    child->build(terrain);
            }*/

            if (!is_leaf(n) 
                && (!isChunk && (n->chunk == nullptr))
                && (!is_materialized(n) || /*if is above of min lod of chunks*/ /*n->depthLOD > octreeSys->minChunkLod)*/) {
                    for (auto* c : n->children) q.push(c);
            }

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
    }

private:
    void QueueChunk(OctreeNode* n) { 
        if (n->isEnqueued) return;
        n->isEnqueued = true;
        cmptChunkScheduler.enqueue(n);
    }

    uint16_t computeBounds(OctreeNode* n) {
        static const glm::vec3 dir[6] = {
            {+1,0,0}, {-1,0,0},
            {0,+1,0}, {0,-1,0},
            {0,0,+1}, {0,0,-1}
        };

        uint16_t b = 0;
        float el = n->edge_length(octreeSys->octreeScale); // edge_length

        for (int i = 0; i < 6; i++) {
            glm::vec3 p = n->center + dir[i] * el;
            int neighLOD = octreeSys->lod_at(p);

            b |= (n->depthLOD < neighLOD ? 1 : 0) << i;        // high→low      0 - 6
            b |= (n->depthLOD > neighLOD ? 1 : 0) << (i + 8);  // low→high      8 - 14
        }
        return b;
    }   
     
    void subdivide(OctreeNode* n) {
        if (n->depthLOD <= octreeSys->maxDepthLod || n->subdivided) return; 
        
        int   childLOD      = n->depthLOD - 1;                                  //float half          = n->size * 0.5f;
        float childOffset   = n->edge_length(octreeSys->octreeScale) * 0.5f;    //* 0.25f

        for (int i = 0; i < 8; i++) { 
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

            glm::vec3 offs = {
                (i & 1) ? +childOffset * 0.5f : -childOffset * 0.5f,
                (i & 2) ? +childOffset * 0.5f : -childOffset * 0.5f,
                (i & 4) ? +childOffset * 0.5f : -childOffset * 0.5f
            };

            n->children[i] = new OctreeNode{
                n->center + offs,
                childLOD
            };
        } 
        n->subdivided = true;
    }   

    void removeChunk(OctreeNode* n) {
        if (is_any_children_enqueued(n) || is_parent_enqueued(n)) return; 
        //if (n->chunk != nullptr) _chunk->queue_free(); 
        n->chunk = nullptr;
    }

    bool is_materialized(OctreeNode* n) { return n->materialized == 0xFF; }
     
    void materialize(OctreeNode* n) {
        if (is_materialized(n)) return;

        if (!n->subdivided) n->materialized = 0xFF;
        else {
            uint8_t mask = 0;
            for (int i = 0; i < 8; i++) {
                if (is_materialized(n->children[i]))
                    mask |= (1 << i);
            }
            n->materialized = mask;
        }
        // propagate to parent if fully materialized
        if (is_materialized(n) && n->father) materialize(n->father);
    }

    // Check if _children is null
    bool is_leaf(OctreeNode* n) const { return !n->subdivided; }
    bool is_parent_enqueued(OctreeNode* n) { return n->father == nullptr ? false : n->father->isEnqueued; }
    bool is_any_children_enqueued(OctreeNode* n) {
        if (is_leaf(n)) return false;
        for (const auto& child : n->children)  if (child->isEnqueued) return true; 
        return false;
    }
      
    void destroy(OctreeNode* n) {
        if (!n) return;
        if (n->subdivided) for (auto* c : n->children) destroy(c);
        delete n;
    }
    void merge(OctreeNode* n) {
        for (auto*& c : n->children) {
            destroy(c);
            c = nullptr;
        }
        n->subdivided = false;
    }
};














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