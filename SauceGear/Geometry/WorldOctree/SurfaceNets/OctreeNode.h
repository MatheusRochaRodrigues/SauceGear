#pragma once
#include "../Math/AABB.h" 
#include <glm/glm.hpp>
#include "SysVoxel.h" 

struct OctreeNode {
    glm::vec3                   center;
    //glm::ivec3                  cell;               // coordenada inteira na grade base  

    int                         depthLOD = 0;       //  0 for the most detailed chuck
    int                         desiredLOD = 0;     //  target Lod in current Shell  

    bool                        subdivided = false;
    std::array<OctreeNode*, 8>  children = { nullptr };
    OctreeNode*                 father = nullptr; 

    //Mesh 
    std::unique_ptr<Chunk>      chunk;
    AABB b; //bounds

    // ---------- cache Otimization ----------
    bool                        isAlreadyPass = false;          // já processado (como no GDVoxel)
    bool                        isEnqueued = false;             // se já está enfileirado para gerar mesh
    uint16_t                    bounds = 0;                     // 12 bits (6 highs / 6 lows)
    uint8_t                     materialized = 0;               // 8 bits (cada child materialized)
      

    // informações SDF
    float sdfMin;
    float sdfMax;
    float sdfCenter;
    bool evaluated = false;
    bool hasSurface = false;


    inline float edge_length() const {   return (1 << depthLOD) * syso.BASE_CELL_SIZE; }
     
    inline float cell_size() { return syso.BASE_CELL_SIZE * (1 << depthLOD); }

    inline AABB getBounds() const {
        auto halfEdge = glm::vec3(edge_length() * 0.5f);
        return AABB(center - halfEdge, center + halfEdge);
    }

    bool isChunk() { return (depthLOD == (desiredLOD + sysv.get_MinChunkLod())); }
    // Check if _children is null
    bool is_leaf() const { return !subdivided; } 

    //talvez
    bool contains(const glm::vec3& p) const
    {
        float half = edge_length() * 0.5f;

        glm::vec3 min = center - glm::vec3(half);
        glm::vec3 max = center + glm::vec3(half);

        return (p.x >= min.x && p.x <= max.x &&
            p.y >= min.y && p.y <= max.y &&
            p.z >= min.z && p.z <= max.z);
    }

    ivec3 World_to_GridChunk(vec3 p) {
        int base = sysv.get_baseChunkSize();
        return ivec3(glm::floor(p.x/base), glm::floor(p.y / base), glm::floor(p.z / base));
    }

    ivec3 snap_World_to_GridChunk(vec3 p) {
        int base = sysv.get_baseChunkSize();
        return ivec3(glm::floor(p.x / base), glm::floor(p.y / base), glm::floor(p.z / base)) * base;
    }
};



//float variation = sdfMax - sdfMin;
//float curvature = variation / node_size;
