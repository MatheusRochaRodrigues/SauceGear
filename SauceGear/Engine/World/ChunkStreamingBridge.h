#pragma once
#include <unordered_set>
#include <queue>
#include <mutex>

#include "../Utils/Threads/ThreadSafeQueue.h"
#include "Chunk/Chunk.h"

//Forward Declaration
//struct Chunk;

struct ChunkStreamingBridge
{
    ThreadSafeQueue<std::shared_ptr<Chunk>> readyChunks;      //readyQueue
    ThreadSafeQueue<std::shared_ptr<Chunk>> removeChunks;    
};

 
struct Bridge {
    // CHUNK ECS 
    ChunkStreamingBridge* ECSBridge;
    std::priority_queue<ChunkRequest>               requestQueue;
    std::unordered_set<ChunkKey, ChunkKeyHasher>    pending;

    std::mutex mtx;
};