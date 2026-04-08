#pragma once
#include <iostream>
#include <atomic>
#include "BlockPool.h" 

template<typename T, int BlockSize = 4096>
class ThreadArena
{
    using Pool = BlockPool<T, BlockSize>;

    struct Block
    {
        T data[BlockSize];
        std::atomic<int> offset{ 0 };
        Block* next = nullptr;
    };

    Block* current = nullptr;
    Pool* pool;

public:

    ThreadArena(Pool* p) : pool(p) {}

    T* Allocate()   //linear memory
    {
        if (!current || current->offset.load() >= BlockSize)    //   //std::memory_order_relaxed
        { 
            Block* newBlock = pool->Acquire();
            newBlock->next = current;
            current = newBlock;
        }

        int index = current->offset.fetch_add(1, std::memory_order_relaxed);    //lock-free, linear, cache-friendly, without malloc (very heavy)
        return &current->data[index];
    }

    // Free All, O(n) without individual destructor, and not fragmentation
    void Reset()
    {
        Block* b = current;

        while (b)
        {
            Block* next = b->next;
            pool->Release(b);
            b = next;
        }

        current = nullptr;
    }
};
 