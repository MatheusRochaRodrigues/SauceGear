#include "WorldController.h"  

namespace DataWorld {
    /*
    int ChunkWorldSize(int lod) {
        return CHUNK_SIZE * BASE_CELL_SIZE * (1 << lod);
    }

    float VoxelSize(int lod) {
        return BASE_CELL_SIZE * (1 << lod);
    }
    */
     

    inline float getVoxelSize(int lod) {
        //return BASE_CELL_SIZE * (1 << lod);        //BASE_VOXEL_SIZE
        return BASE_CELL_SIZE << lod;       
    }

    float ChunkWorldSize(int lod) {
        return GRID_RESOLUTION * getVoxelSize(lod);
    } 
     
    int ChunkGridSize(int lod) {
        return GRID_RESOLUTION * (1 << lod);
    } 

    inline glm::vec3 gridToWorld(const glm::ivec3& gridPos, int lod)
    {
        return glm::vec3(gridPos) * getVoxelSize(lod);
    }
    
}