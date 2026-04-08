#pragma once
#include "../Utils/Threads/JobSystem.h" 
#include "../Utils/Threads/ThreadArena.h" 
#include "../Geometry/Voxel/dual_octree_mesh/Data/DCNode.h" 

// JOBSYSTEM
extern JobSystem global_JobSystem; 

// GLOBAL MEMORY
extern BlockPool<DCNode, 4096> global_NodePool;  

extern ThreadArena<DCNode>& GetNodeArena();



//struct ChunkMemory
//{
//    ThreadArena<DCNode> nodeArena; 
//    ChunkMemory() : nodeArena(&global_NodePool) {}
//
//    ~ChunkMemory() {
//        nodeArena.Reset();
//        //delete this/*memory*/;
//    }
//};