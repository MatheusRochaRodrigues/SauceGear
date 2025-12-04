#pragma once
#include "../Utils/Bounds.h" 
#include <glm/glm.hpp>
#include "SysVoxel.h" 

struct OctreeNode {
    glm::vec3                   center;
    int                         depthLOD = 0;           //  0 for the most detailed chuck
    int                         desiredLOD = 0;     //  target Lod in current Shell 
    Bounds                      b;

    bool                        subdivided = false;
    std::array<OctreeNode*, 8>  children = { nullptr };
    OctreeNode*                 father = nullptr;
    float                       distSurf_SDF = 0;

    //Mesh 
    std::unique_ptr<Chunk>      chunk;

    // ---------- cache Otimization ----------
    bool                        isAlreadyPass = false;         // já processado (como no GDVoxel)
    bool                        isEnqueued = false;   // se já está enfileirado para gerar mesh
    uint16_t                    bounds = 0;      // 12 bits (6 highs / 6 lows)
    uint8_t                     materialized = 0;   // 8 bits (cada child materialized)



    // informações SDF
    float sdfMin;
    float sdfMax;
    float sdfCenter;
    bool evaluated = false;
    bool hasSurface = false;

    inline float edge_length(float scale = 0) const {
        if (scale == 0) scale = syso.octreeScale;
        return (1 << depthLOD) * scale;
    } //_size

    inline Bounds getBounds(float scale = 0) const {
        if (scale == 0) scale = syso.octreeScale;
        auto halfEdge = glm::vec3(edge_length(scale) * 0.5f);
        return Bounds(center - halfEdge, center + halfEdge);
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
};