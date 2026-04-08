#pragma once
#include <glm/glm.hpp>

// ============================================================
// CONFIG
// ============================================================
 
constexpr int   GRID_RESOLUTION  = 32 / 4;            // CHUNK_SIZE == RESOLUTION == Grid
constexpr float BASE_CELL_SIZE   = 1 / 1.0f;				     // float BASE_CELL_SIZE = 1.0f; 
constexpr int   CLIP_LEVELS      = 5;				     
constexpr int   BASE_RING_RADIUS = 2;

namespace DataWorld { 
    /*
    int     ChunkWorldSize(int lod);
    float   VoxelSize(int lod);
    */
     
    int ChunkGridSize(int lod); 
    float ChunkWorldSize(int lod);
     
    inline float getVoxelSize(int lod); 
    inline glm::vec3 gridToWorld(const glm::ivec3& gridPos, int lod); 
     
}