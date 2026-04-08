#pragma once 
#include <iostream>
#include <atomic> 

template<typename T, int BlockSize = 4096>
class BlockPool
{
    struct Block
    {
        T data[BlockSize];
        std::atomic<int> offset{ 0 };
        Block* next = nullptr;
    };

    std::atomic<Block*> freeList{ nullptr };

public:

    Block* Acquire()
    {
        Block* b = freeList.load(std::memory_order_acquire);

        while (b)
        {
            // try removing the block from the list, if outher thread steal it (CAS Failed), then try again (deafult lock-free)
            if (freeList.compare_exchange_weak(
                b, b->next,
                std::memory_order_acquire,
                std::memory_order_relaxed))
            {
                b->offset.store(0, std::memory_order_relaxed);  // clean block for recycle
                return b;
            }
        }

        return new Block(); // fallback (it should happen rarely)
    }

    void Release(Block* b)
    {
        Block* head = freeList.load(std::memory_order_relaxed);

        do
        {
            b->next = head;
        } while (!freeList.compare_exchange_weak(
            head,
            b,
            std::memory_order_release,
            std::memory_order_relaxed));
    }
};


/*
* to avoid ABA
struct TaggedPtr
{
    void* ptr;
    uint64_t tag;
};
*/