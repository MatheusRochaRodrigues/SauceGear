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

    // To prevent ABA race conditions between threads, we use a tag along with the block.
    struct TaggedPtr
    {
        Block* ptr;     // 8 bytes
        uint64_t tag;   // 8 bytes
    };                  // 16 bytes in total

    std::atomic<TaggedPtr> freeList;

public:  
    BlockPool()
    { 
        debug();
        freeList.store({ nullptr, 0 });
    } 
    
    Block* Acquire()        // ~Pop
    {
        TaggedPtr oldHead = freeList.load(std::memory_order_acquire);

        while (oldHead.ptr)
        {
            TaggedPtr newHead;
            newHead.ptr = oldHead.ptr->next;
            newHead.tag = oldHead.tag + 1;

            // try removing the block from the list, if outher thread steal it (CAS Failed), then try again (deafult lock-free)
            if (freeList.compare_exchange_weak(
                oldHead, newHead,
                std::memory_order_acq_rel,
                std::memory_order_acquire))
            {
                oldHead.ptr->offset.store(0, std::memory_order_relaxed);     // clean block for recycle
                return oldHead.ptr;
            }
        }
         
        return new Block();     // fallback (it should happen rarely)
    } 

    void Release(Block* b)        // ~Push
    {
        TaggedPtr oldHead = freeList.load(std::memory_order_acquire);

        TaggedPtr newHead;

        do
        {
            b->next = oldHead.ptr;

            newHead.ptr = b;
            newHead.tag = oldHead.tag + 1;

        } while (!freeList.compare_exchange_weak(
            oldHead,
            newHead,
            std::memory_order_acq_rel,
            std::memory_order_acquire));
    }

private: 
    void debug() {
        // verifica se é lock-free em tempo de compilação
        if constexpr (std::atomic<TaggedPtr>::is_always_lock_free) {
            std::cout << "TaggedPtr is lock-free!\n";
        }
        else {
            std::cout << "TaggedPtr is NOT lock-free! (may use internal mutex)\n";
        }

        return;
        // opcional: força compilação falhar se não for lock-free
        static_assert(std::atomic<TaggedPtr>::is_always_lock_free,
            "TaggedPtr must be lock-free for proper concurrency!");
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


/*
struct TaggedPtr
{
    void* ptr;
    uint64_t tag;

    bool operator==(const TaggedPtr& other) const
    {
        return ptr == other.ptr && tag == other.tag;
    }
};
*/