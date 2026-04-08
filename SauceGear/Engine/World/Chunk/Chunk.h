#pragma once  
#include <iostream>
#include <glm/glm.hpp>
#include "../../Graphics/Vertex.h" 
#include "ChunkTypes.h"  
//#include "../C_Thread.h"    

using namespace glm;

// FORWARD DECLARATIONS
class DCNode;
class DensityCache;
struct Bridge;
//struct ChunkMemory;

enum class ChunkState : uint8_t
{
    Empty,
    NeedsBuild,
    Building,
    MeshReady,
    Stitched,
    Ready
};


struct alignas(64) Chunk
{
    ivec3                           coord;
    int                             lod;
    std::atomic<ChunkState>         state = ChunkState::Empty;
    DCNode*                         root = nullptr;    // std::unique_ptr<DCNode> 
    VertexBuffer                    vertexBuffer;
    IndexBuffer                     indexBuffer;
     
    //Threads
    std::atomic<bool>               pendingRemoval = false; 
    //std::shared_ptr<ChunkMemory>    memory;
}; 

// ============================================================
// FUNCTIONS
// ============================================================

void BuildChunk(std::shared_ptr<Chunk> c, DensityCache& densityCache, Bridge* bridge);
