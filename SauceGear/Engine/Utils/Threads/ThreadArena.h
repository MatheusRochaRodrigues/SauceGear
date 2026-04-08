#pragma once
#include <iostream>
#include <atomic>
#include "BlockPool.h"

template<typename T, int BlockSize = 4096>
class ThreadArena
{
    using Pool = BlockPool<T, BlockSize>;
    using Block = typename Pool::Block;

    Block* current = nullptr;
    Pool* pool;

public:

    ThreadArena(Pool* p) : pool(p) {}

    T* Allocate()   //linear memory
    {
        if (!current || current->offset.load(std::memory_order_relaxed) >= BlockSize)    //maybe use -> "std::memory_order_relaxed" in load
        { 
            Block* newBlock = pool->Acquire();
            newBlock->next = current;
            current = newBlock;
        }

        int index = current->offset.fetch_add(1, std::memory_order_relaxed);    //lock-free, linear, cache-friendly, without malloc (very heavy)
        //return &current->data[index];     // -LEGACY

        T* ptr = &current->data[index];
        new (ptr) T();

        return ptr;
    }

    // Free All, O(n) without individual destructor, and not fragmentation
    void Reset()
    {
        Block* b = current;

        while (b)
        {
            Block* next = b->next;
            pool->Release(b);           //b->data[i].~T();
            b = next;
        }

        current = nullptr;
    }
};
 
/*placement new*/