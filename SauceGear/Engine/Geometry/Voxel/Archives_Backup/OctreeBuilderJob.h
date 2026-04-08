#pragma once
#include <functional>
#include <glm/glm.hpp>  
//Threads
#include "DCNode.h" 
#include "../../../Utils/Threads/ThreadArena.h"  
#include "../../../Utils/Threads/JobSystem.h"

struct ChunkMemory
{
    ThreadArena<DCNode> nodeArena;
    //ThreadArena<DCNode>* nodeArena; // ponteiro

    ChunkMemory(BlockPool<DCNode>* pool) : nodeArena(pool) {}
};
 
void BuildOctreeParallel(const glm::ivec3 min, int size, BuildCxt* ctx, std::function<void(DCNode*)>); 
