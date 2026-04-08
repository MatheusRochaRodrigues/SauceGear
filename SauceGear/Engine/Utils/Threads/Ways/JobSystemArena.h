#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <condition_variable>
#include "ThreadArena.h"

// CONFIG
constexpr int CHAIN_BATCH = 4;
constexpr int STEAL_BATCH = 16;

// ============================================================
// TASK
// ============================================================

struct Task
{
    void (*function)(Task*, void*) = nullptr;
    void* data = nullptr;

    std::atomic<int> dependencies{ 0 };

    std::vector<Task*> dependents;
    std::mutex dependentsMutex;

    std::atomic<bool> enqueued{ false };
};

// ============================================================
// GLOBAL POOL
// ============================================================

template<typename T, int BlockSize>
class BlockPool;

extern BlockPool<Task, 2048> g_taskPool;

// ============================================================
// WORK STEALING QUEUE
// ============================================================

class WorkStealingQueue
{
public:

    void Push(Task* t)
    {
        std::lock_guard<std::mutex> lock(mtx);
        dq.push_back(t);
    }

    bool Pop(Task*& out)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (dq.empty()) return false;

        out = dq.back();
        dq.pop_back();
        return true;
    }

    int StealBatch(std::vector<Task*>& out, int maxCount)
    {
        std::lock_guard<std::mutex> lock(mtx);

        int count = 0;
        while (!dq.empty() && count < maxCount)
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

private:
    std::deque<Task*> dq;
    std::mutex mtx;
};

// ============================================================
// JOB SYSTEM
// ============================================================

class JobSystem
{
public:

    JobSystem(int threads = std::thread::hardware_concurrency())
    {
        Init(threads);
    }

    ~JobSystem()
    {
        stop.store(true);
        cv.notify_all();

        for (auto& t : workers)
            t.join();
    }

    void Init(int n)
    {
        queues.resize(n);

        for (int i = 0; i < n; i++)
        {
            queues[i] = std::make_unique<WorkStealingQueue>();

            workers.emplace_back([this, i]()
                {
                    Worker(i);
                });
        }
    }

    // ========================================================
    // TASK CREATION (POOL)
    // ========================================================

    Task* CreateTask(void (*fn)(Task*, void*), void* data)
    {
        Task* t = g_taskPool.Acquire();
        new (t) Task();

        t->function = fn;
        t->data = data;
        t->dependencies.store(0, std::memory_order_relaxed);
        t->enqueued.store(false, std::memory_order_relaxed);

        t->dependents.reserve(4); // 🔥 evita realloc

        return t;
    }

    // ========================================================
    void AddDependency(Task* parent, Task* child)
    {
        child->dependencies.fetch_add(1, std::memory_order_relaxed);

        std::lock_guard<std::mutex> lock(parent->dependentsMutex);
        parent->dependents.push_back(child);
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
        int idx = GetThreadIndex();
        queues[idx]->Push(t);

        cv.notify_one();
    }

    // ========================================================
    void Worker(int index)
    {
        threadIndex = index;

        std::vector<Task*> batch;
        batch.reserve(32);

        while (!stop.load(std::memory_order_relaxed))
        {
            Task* task = nullptr;

            // local
            if (queues[index]->Pop(task))
            {
                ExecuteBatch(task, index);
                continue;
            }

            // steal
            if (Steal(index, batch))
            {
                for (Task* t : batch)
                    ExecuteBatch(t, index);

                batch.clear();
                continue;
            }

            // sleep
            std::unique_lock<std::mutex> lock(cvMutex);
            cv.wait(lock, [this]()
                {
                    return stop.load() || HasWork();
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

            if (queues[i]->StealBatch(out, STEAL_BATCH) > 0)
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

            Task* next = nullptr;
            if (!queues[index]->Pop(next))
                break;

            current = next;
        }
    }

    // ========================================================
    void Finish(Task* task)
    {
        std::vector<Task*> deps;

        {
            std::lock_guard<std::mutex> lock(task->dependentsMutex);
            deps = std::move(task->dependents);
        }

        for (Task* d : deps)
        {
            if (d->dependencies.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                TrySchedule(d);
            }
        }

        task->~Task();
        g_taskPool.Release(task);
    }

    // ========================================================
    int GetThreadIndex()
    {
        if (threadIndex < 0) return 0;
        return threadIndex % queues.size();
    }

private:

    std::vector<std::thread> workers;
    std::vector<std::unique_ptr<WorkStealingQueue>> queues;

    std::atomic<bool> stop{ false };

    std::condition_variable cv;
    std::mutex cvMutex;

    static thread_local int threadIndex;
};