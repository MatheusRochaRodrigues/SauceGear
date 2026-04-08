#pragma once
#include <iostream>
#include <atomic>

template<typename T, int BlockSize = 4096>
class ThreadArena
{
    struct Block
    {
        T data[BlockSize];
        std::atomic<int> offset{ 0 };
        Block* next = nullptr;
    };

    std::atomic<Block*> head{ nullptr };

public:

    T* Allocate()
    {
        Block* b = head.load(std::memory_order_acquire);

        if (!b || b->offset.load(std::memory_order_relaxed) >= BlockSize)
        {
            Block* newBlock = new Block();
            newBlock->next = b;

            while (!head.compare_exchange_weak(
                b, newBlock,
                std::memory_order_release,
                std::memory_order_acquire))
            {
                newBlock->next = b;
            }

            b = newBlock;
        }

        int index = b->offset.fetch_add(1, std::memory_order_relaxed);
        return &b->data[index];
    }
};