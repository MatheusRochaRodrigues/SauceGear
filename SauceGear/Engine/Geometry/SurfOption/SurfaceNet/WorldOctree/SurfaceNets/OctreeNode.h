#pragma once
#include <glm/glm.hpp>
#include "SysVoxel.h" 
#include <array>
#include "../../../../../Math/AABB.h" 
//#include "../Geometry/World/SurfaceNets/SurfaceNets.h" 

struct Chunk; // forward, já que usa unique_ptr

struct OctreeNode {
    glm::vec3                   center;
    //glm::ivec3                  cell;               // coordenada inteira na grade base  

    int                         depthLOD = 0;       //  0 for the most detailed chuck
    int                         targetLOD = 0;     //  target Lod in current Shell  

    OctreeNode*                 father = nullptr;
    bool                        subdivided = false;
    std::array<OctreeNode*, 8>  children = { nullptr };

    //Mesh 
    std::unique_ptr<Chunk>      chunk;
    AABB b; //bounds

    // ---------- cache Otimization ----------
    bool                        isEvaluated = false;          // já processado (como no GDVoxel)
    bool                        isEnqueued = false;             // se já está enfileirado para gerar mesh
    uint16_t                    bounds = 0;                     // 12 bits (6 highs / 6 lows)
    uint8_t                     materialized = 0;               // 8 bits (cada child materialized)
       
    float sdfCenter; 
    bool isDirty = true;
    glm::vec3 sdfGradient;

    inline glm::vec3 grid_origin() const {
        float size = edge_length();        return glm::floor(center / size) * size;
    }

    inline float edge_length() const { return (1 << depthLOD) * syso.BASE_CELL_SIZE; }
     
    inline float voxel_size() { return edge_length() / sysv.get_cellGrid(); }              //inline float cell_size() { return syso.BASE_CELL_SIZE * (1 << depthLOD); }

    inline AABB getBounds() const {
        auto halfEdge = glm::vec3(edge_length() * 0.5f);
        return AABB(center - halfEdge, center + halfEdge);
    }

    bool isChunk() { return (depthLOD == ( targetLOD + sysv.get_MinChunkLod() )); }

    // Check if _children is null
    bool is_leaf() const { return !subdivided; } 

    //talvez
    bool contains(const glm::vec3& p) const {
        float half = edge_length() * 0.5f; 
        glm::vec3 min = center - glm::vec3(half);    glm::vec3 max = center + glm::vec3(half);

        return (p.x >= min.x && p.x <= max.x &&
                p.y >= min.y && p.y <= max.y &&
                p.z >= min.z && p.z <= max.z );
    }

    float get_value() {
        if (!isDirty) return sdfCenter;      //cuidado aq antes era _value

        if (!is_leaf()) {
            sdfCenter = 0; 
            for (auto& child : children) sdfCenter += child->get_value();  
            sdfCenter *= 0.125f;    //same / 8
        }

        isDirty = false;
        return sdfCenter;
    }
    
    
};






/*

// todo, make threadsafe. It's currently maybe fine (due to the way the scheduler is set up), but better safe than
// sorry.
    float get_value()
    {
        if (!is_dirty())
            return _value;      //cuidado aq antes era _value
        if (!is_leaf())
        {
            _value = 0;
            NodeColor = glm::vec4(0, 0, 0, 0);
            for (auto& child : (*_children))
            {
                _value += child->get_value();
                NodeColor += child->NodeColor;
            }
            _value *= 0.125f;
            NodeColor *= 0.125f;
        }
        _isDirty = false;
        return _value;
    }

*/



/*
glm::ivec3 World_to_GridChunk(glm::vec3 p) {
    int base = sysv.get_baseChunkSize();
    return glm::ivec3(glm::floor(p.x / base), glm::floor(p.y / base), glm::floor(p.z / base));
}

glm::ivec3 snap_World_to_GridChunk(glm::vec3 p) {
    int base = sysv.get_baseChunkSize();
    return glm::ivec3(glm::floor(p.x / base), glm::floor(p.y / base), glm::floor(p.z / base)) * base;
}
*/


//float variation = sdfMax - sdfMin;
//float curvature = variation / node_size;
