#pragma once 

/*
// Forward declaration de DCNode
struct DCNode;

// Forward declaration de ThreadArena
template<typename T>
class ThreadArena;

struct ChunkMemory
{
    ThreadArena<DCNode>* nodeArena;  // ponteiro -> ok com forward declaration

    ChunkMemory() : nodeArena(nullptr) {}
};
*/


/*
struct DCNode;  // forward declaration

template<typename T, int>
class ThreadArena;  // forward declaration do template

struct ChunkMemory
{
    ThreadArena<DCNode>* nodeArena;

    ChunkMemory(BlockPool<DCNode>* pool)
    {
        nodeArena = new ThreadArena<DCNode>(pool); // precisa da definição de ThreadArena aqui
    }
*/


/*
struct ChunkMemory
{
    ThreadArena<DCNode> nodeArena;                  //thread_local ThreadArena<DCNode, 4096> arena(&g_nodePool);    //cada thread tem sua arena

    ChunkMemory() : nodeArena(&global_NodePool) {}
};
*/