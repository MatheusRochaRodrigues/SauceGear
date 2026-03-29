#pragma once
#include "../Utils/Threads/ThreadSafeQueue.h"
#include "Chunk/Chunk.h"

//Forward Declaration
//struct Chunk;

struct ChunkStreamingBridge
{
    ThreadSafeQueue<std::shared_ptr<Chunk>> readyChunks;      //readyQueue
    ThreadSafeQueue<std::shared_ptr<Chunk>> removeChunks;    
};