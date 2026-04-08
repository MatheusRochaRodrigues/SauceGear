#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <condition_variable>

// ============================================================
// CONFIG
// ============================================================

constexpr int CHAIN_BATCH = 4;
constexpr int MAX_STEAL_BATCH = 16;
constexpr int RESERVE_BATCH = 32;

// ============================================================
// BLOCK POOL (GLOBAL)
// ============================================================

template<typename T, int BlockSize = 2048>
class BlockPool
{
public:
    struct Block
    {
        T data[BlockSize];
        std::atomic<int> offset{ 0 };
        Block* next = nullptr;
    };

private:
    std::atomic<Block*> freeList{ nullptr };

public:

    Block* Acquire()
    {
        Block* b = freeList.load(std::memory_order_acquire);

        while (b)
        {
            if (freeList.compare_exchange_weak(
                b, b->next,
                std::memory_order_acquire,
                std::memory_order_relaxed))
            {
                b->offset.store(0, std::memory_order_relaxed);
                return b;
            }
        }

        return new Block();
    }

    void Release(Block* b)
    {
        Block* head = freeList.load(std::memory_order_relaxed);

        do
        {
            b->next = head;
        } while (!freeList.compare_exchange_weak(
            head, b,
            std::memory_order_release,
            std::memory_order_relaxed));
    }
};

// ============================================================
// THREAD ARENA
// ============================================================

template<typename T, int BlockSize = 2048>
class ThreadArena
{
    using Pool = BlockPool<T, BlockSize>;
    using Block = typename Pool::Block;

    Block* current = nullptr;
    Pool* pool;

public:

    ThreadArena(Pool* p) : pool(p) {}

    T* Allocate()
    {
        if (!current || current->offset.load(std::memory_order_relaxed) >= BlockSize)
        {
            Block* b = pool->Acquire();
            b->next = current;
            current = b;
        }

        int index = current->offset.fetch_add(1, std::memory_order_relaxed);

        T* ptr = &current->data[index];
        new (ptr) T();

        return ptr;
    }

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

// ============================================================
// TASK
// ============================================================

struct Task
{
    void (*function)(Task*, void*) = nullptr;
    void* data = nullptr;

    std::atomic<int> dependencies{ 0 };

    Task* nextDependent = nullptr; // 🔥 intrusive list (SEM vector)
    std::atomic<Task*> dependents{ nullptr };

    std::atomic<bool> enqueued{ false };
};

// ============================================================
// QUEUE
// ============================================================

class WorkStealingQueue
{
    std::deque<Task*> dq;
    std::mutex mtx;

public:

    void Push(Task* t)
    {
        std::lock_guard<std::mutex> lock(mtx);
        dq.push_back(t);
    }

    bool Pop(Task*& t)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (dq.empty()) return false;

        t = dq.back();
        dq.pop_back();
        return true;
    }

    int StealBatch(std::vector<Task*>& out, int max)
    {
        std::lock_guard<std::mutex> lock(mtx);

        int count = 0;
        while (!dq.empty() && count < max)
        {
            out.push_back(dq.front());
            dq.pop_front();
            count++;
        }

        return count;
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return dq.empty();
    }
};

// ============================================================
// JOB SYSTEM
// ============================================================

class JobSystem
{
public:

    JobSystem(int threads = std::thread::hardware_concurrency() / 2)
    {
        Init(threads);
    }

    ~JobSystem()
    {
        stop = true;
        cv.notify_all();

        for (auto& t : workers)
            t.join();
    }

    // ========================================================

    Task* CreateTask(void (*fn)(Task*, void*), void* data)
    {
        Task* t = GetArena().Allocate();

        t->function = fn;
        t->data = data;
        t->dependencies.store(0);
        t->dependents.store(nullptr);
        t->enqueued.store(false);

        return t;
    }

    // ========================================================

    void AddDependency(Task* parent, Task* child)
    {
        child->dependencies.fetch_add(1, std::memory_order_relaxed);

        // 🔥 intrusive lock-free push
        Task* head = parent->dependents.load();

        do
        {
            child->nextDependent = head;
        } while (!parent->dependents.compare_exchange_weak(head, child));
    }

    // ========================================================

    void TrySchedule(Task* t)
    {
        if (t->dependencies.load(std::memory_order_acquire) != 0)
            return;

        bool expected = false;
        if (!t->enqueued.compare_exchange_strong(expected, true))
            return;

        Push(t);
    }

private:

    void Push(Task* t)
    {
        queues[GetThreadIndex()]->Push(t);
        cv.notify_one();
    }

    // ========================================================

    void Worker(int index)
    {
        threadIndex = index;

        std::vector<Task*> batch;
        batch.reserve(RESERVE_BATCH);

        while (!stop)
        {
            Task* t = nullptr;

            if (queues[index]->Pop(t))
            {
                ExecuteBatch(t, index);
                continue;
            }

            if (Steal(index, batch))
            {
                for (Task* job : batch)
                    ExecuteBatch(job, index);

                batch.clear();
                continue;
            }

            std::unique_lock<std::mutex> lock(cvMutex);
            cv.wait(lock, [this]()
                {
                    return stop || HasWork();
                });
        }
    }

    bool HasWork()
    {
        for (auto& q : queues)
            if (!q->Empty()) return true;

        return false;
    }

    bool Steal(int index, std::vector<Task*>& out)
    {
        for (int i = 0; i < queues.size(); i++)
        {
            if (i == index) continue;

            if (queues[i]->StealBatch(out, MAX_STEAL_BATCH))
                return true;
        }
        return false;
    }

    // ========================================================

    void ExecuteBatch(Task* first, int index)
    {
        Task* current = first;

        for (int i = 0; i < CHAIN_BATCH && current; i++)
        {
            current->function(current, current->data);
            Finish(current);

            current = nullptr;
            queues[index]->Pop(current);
        }

        if (current)
            TrySchedule(current);
    }

    void Finish(Task* t)
    {
        Task* dep = t->dependents.load();

        while (dep)
        {
            Task* next = dep->nextDependent;

            if (dep->dependencies.fetch_sub(1) == 1)
                TrySchedule(dep);

            dep = next;
        }
    }

    // ========================================================

    ThreadArena<Task>& GetArena()
    {
        thread_local ThreadArena<Task> arena(&GetPool());
        return arena;
    }

    BlockPool<Task>& GetPool()
    {
        static BlockPool<Task> pool;
        return pool;
    }

    int GetThreadIndex()
    {
        return threadIndex < 0 ? 0 : threadIndex;
    }

private:

    std::vector<std::thread> workers;
    std::vector<std::unique_ptr<WorkStealingQueue>> queues;

    std::atomic<bool> stop{ false };

    std::condition_variable cv;
    std::mutex cvMutex;

    static thread_local int threadIndex;
};

thread_local int JobSystem::threadIndex = -1;