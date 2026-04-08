 #include "C_Thread.h"
 
JobSystem global_JobSystem;

BlockPool<DCNode, 4096> global_NodePool;

ThreadArena<DCNode>& GetNodeArena()
{
    thread_local ThreadArena<DCNode> arena(&global_NodePool);
    return arena;
}