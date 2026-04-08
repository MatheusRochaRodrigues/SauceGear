#pragma once
#include <atomic>
#include <cstddef>
#include <new>

template<typename T, int LOCAL_CACHE_SIZE = 64, int BATCH_SIZE = 32>
class ObjectPool
{
    struct Node
    {
        alignas(T) unsigned char storage[sizeof(T)];
        Node* next;

        T* Get() { return reinterpret_cast<T*>(storage); }
    };

    std::atomic<Node*> freeList{ nullptr };

    // =========================
    // THREAD LOCAL CACHE
    // =========================
    struct LocalCache
    {
        Node* head = nullptr;
        int count = 0;
    };

    static thread_local LocalCache local;

public:

    // =========================================================
    // ACQUIRE
    // =========================================================
    /*
    T* Acquire()
    {
        // FAST PATH (sem atomics)
        if (local.head)
        {
            Node* n = local.head;
            local.head = n->next;
            local.count--;

            return n->Get();
        }

        //  SLOW PATH (pega batch do global)
        RefillLocal();

        if (local.head)
        {
            Node* n = local.head;
            local.head = n->next;
            local.count--;

            return n->Get();
        }

        // fallback (raro)
        Node* n = new Node();
        return n->Get();
    }
    */

    T* Acquire()
    {
        Node* n;

        if (local.head)
        {
            n = local.head;
            local.head = n->next;
            local.count--;
        }
        else
        {
            RefillLocal();

            if (local.head)
            {
                n = local.head;
                local.head = n->next;
                local.count--;
            }
            else
            {
                n = new Node();
            }
        }

        T* obj = n->Get();
        new (obj) T(); //  CONSTRÓI

        return obj;
    }

    // =========================================================
    // RELEASE
    // =========================================================
    void Release(T* obj)
    {
        obj->~T();  // EXTRA

        Node* n = reinterpret_cast<Node*>(obj);

        // adiciona no cache local
        n->next = local.head;
        local.head = n;
        local.count++;

        //  evita crescer infinito
        if (local.count >= LOCAL_CACHE_SIZE)
        {
            FlushLocal();
        }
    }

    // =========================================================
    // PREALLOC (BLOCK)
    // =========================================================
    void PreallocateBlock(size_t count)
    {
        Node* block = (Node*)::operator new(sizeof(Node) * count);

        for (size_t i = 0; i < count; i++)
        {
            Node* n = &block[i];

            PushGlobal(n);
        }
    }

private:

    // =========================================================
    // PUSH GLOBAL (lock-free)
    // =========================================================
    void PushGlobal(Node* n)
    {
        Node* head = freeList.load(std::memory_order_relaxed);

        do {
            n->next = head;
        } while (!freeList.compare_exchange_weak(
            head,
            n,
            std::memory_order_release,
            std::memory_order_relaxed));
    }

    // =========================================================
    // POP BATCH GLOBAL → LOCAL
    // =========================================================
    void RefillLocal()
    {
        Node* batch = nullptr;
        int count = 0;

        while (count < BATCH_SIZE)
        {
            Node* head = freeList.load(std::memory_order_acquire);
            if (!head) break;

            Node* next = head->next;

            if (freeList.compare_exchange_weak(
                head,
                next,
                std::memory_order_acquire,
                std::memory_order_relaxed))
            {
                head->next = batch;
                batch = head;
                count++;
            }
        }

        // move batch pro cache local
        while (batch)
        {
            Node* next = batch->next;

            batch->next = local.head;
            local.head = batch;
            local.count++;

            batch = next;
        }
    }

    // =========================================================
    // FLUSH LOCAL → GLOBAL
    // =========================================================
    void FlushLocal()
    {
        int count = 0;

        while (local.head && count < BATCH_SIZE)
        {
            Node* n = local.head;
            local.head = n->next;

            PushGlobal(n);

            local.count--;
            count++;
        }
    }
};

// definição do thread_local
template<typename T, int LOCAL_CACHE_SIZE, int BATCH_SIZE>
thread_local typename ObjectPool<T, LOCAL_CACHE_SIZE, BATCH_SIZE>::LocalCache
ObjectPool<T, LOCAL_CACHE_SIZE, BATCH_SIZE>::local;
 