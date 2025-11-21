#pragma once
#include <array>
#include <queue>
#include <glm/glm.hpp>
#include "LODController.h" 
#include "Map.h" 

struct OctreeNode {
    OctreeNode* father = nullptr;
    glm::vec3 center;
    //float size;     
    unsigned int depthLOD;      //lod        0 for the most detailed chuck
    bool subdivided = false;
    std::array<OctreeNode*, 8> children = { nullptr };

    std::unique_ptr<Chunk> chunk;
    uint16_t boundaries = 0;

    // ---------- ADICIONAR ----------
    bool isAlreadyPass = false;         // já processado (como no GDVoxel)
    bool isEnqueued = false;   // se já está enfileirado para gerar mesh
    uint16_t bounds = 0;      // 12 bits (6 highs / 6 lows)
    uint8_t materialized = 0;   // 8 bits (cada child materialized)
    float distSurf_SDF;
};

class OctreeLOD {
public:
    OctreeNode* root = nullptr;
    LODController* lodSystem;

    Map map;

    OctreeLOD(const glm::vec3& center, float size, LODController* l) : lodSystem(l) {
        root = new OctreeNode{ center, size, l->lodCount - 1 };
    } 
    ~OctreeLOD() { destroy(root); }  

    void update_node() { 
        int desiredLOD = lodSystem->lod_at(n->center);
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
            if (n->size > lodSystem->lod_grid_size(desiredLOD)) {
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

    void update() {
        std::queue<OctreeNode*> q;  q.push(root);

        while (!q.empty()) {
            OctreeNode* n = q.front(); q.pop();
            //verifica em qual shell está
            int desiredLOD = lodSystem->lod_at(n->center);      // equivalent to the targetLOD 
            if (desiredLOD < 0) return;

            if (!n->isAlreadyPass) {  //cache interno do octree   
                // --- SDF Distance ---
                float sdf = map.sdf->sdfDistance(n->center); n->distSurf_SDF = sdf;
                bool notArrivedLod = (n->depthLOD > desiredLOD);

                //if exists zero-crossing of SDF into of the node  &&  if the current lod of the node have desired Lod based player position
                if (map->has_surface(n->center, sdf) && notArrivedLod) {
                    if (!n->subdivided) subdivide(n); 
                    for (auto* c : n->children) q.push(c);
                    n->isAlreadyPass = true;
                }

                // if we don't subdivide further, we mark it as a fully realized subtree
                if (is_leaf() && (notArrivedLod || n->depthLOD == lodSystem->minLod /*0*/)) {
                    n->isAlreadyPass = true;
                    materialize(n);
                    continue;
                } else {
                    if (n->subdivided) merge(n);
                } 
            }

            bool isChunk = (n->depthLOD == (desiredLOD + terrain.get_min_chunk_size()));
            if (is_chunk(terrain) && !is_leaf() && (n->chunk == nullptr || (n->bounds != computeBounds(terrain))))
                queue_update(terrain);

            if (!is_leaf() && !(is_chunk(terrain) && (n->chunk != nullptr)) && // || is_enqueued()
                (!is_materialized() || is_above_min_chunk(terrain)))
                for (auto& child : *_children)
                    child->build(terrain);

            //falta por delet

        }


    }

private:
    uint16_t computeBounds(OctreeNode* n) {
        static const glm::vec3 dir[6] = {
            {+1,0,0}, {-1,0,0},
            {0,+1,0}, {0,-1,0},
            {0,0,+1}, {0,0,-1}
        };

        uint16_t b = 0;
        float el = n->size; // edge_length

        for (int i = 0; i < 6; i++) {
            glm::vec3 p = n->center + dir[i] * el;
            int neighLOD = lodSystem->lod_at(p);

            b |= (n->lod < neighLOD ? 1 : 0) << i;      // high→low
            b |= (n->lod > neighLOD ? 1 : 0) << (i + 8);  // low→high
        }

        return b;
    }   

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
    bool is_materialized(OctreeNode* n) { return n->materialized == 0xFF; }

    void subdivide(OctreeNode* n) {
        float half = n->size * 0.5f;
        int childLOD = n->lod - 1;

        for (int i = 0; i < 8; i++) {
            glm::vec3 offs = {
                (i & 1) ? +half * 0.5f : -half * 0.5f,
                (i & 2) ? +half * 0.5f : -half * 0.5f,
                (i & 4) ? +half * 0.5f : -half * 0.5f
            };

            n->children[i] = new OctreeNode{
                n->center + offs,
                half,
                childLOD
            };
        }

        n->subdivided = true;
    }

    void merge(OctreeNode* n) {
        for (auto*& c : n->children) {
            destroy(c);
            c = nullptr;
        }
        n->subdivided = false;
    }

    void destroy(OctreeNode* n) {
        if (!n) return;
        if (n->subdivided) for (auto* c : n->children) destroy(c);
        delete n;
    }


    void removeChunk(OctreeNode* n) {
        if (is_any_children_enqueued(n) || is_parent_enqueued(n)) return; 
        //if (n->chunk != nullptr) _chunk->queue_free(); 
        n->chunk = nullptr;
    }

    // Check if _children is null
    bool is_leaf() const { return !subdivide; }
    bool is_parent_enqueued(OctreeNode* n) { return n->father == nullptr ? false : n->father->isEnqueued; }
    bool is_any_children_enqueued(OctreeNode* n) {
        if (is_leaf()) return false;
        for (const auto& child : n->children)  if (child->isEnqueued) return true; 
        return false;
    }
};





/*
void update2() {
    std::queue<OctreeNode*> q;
    q.push(root);

    while (!q.empty()) {
        OctreeNode* n = q.front(); q.pop();

        //verifica em qual shell está
        int targetLOD = lodSystem->lod_at(n->center);

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