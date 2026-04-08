#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <condition_variable>
#include "JLOG.h"
  
#define chainBatchRun 8/2 
#define reserveBatch  64 
#define maxStealBatch 16  

// ============================================================
// TASK
// ============================================================

//template<typename T>      void* to T
struct Task
{
    void (*function)(Task*, void*) = nullptr;
    void* data = nullptr;

    std::atomic<int> dependencies{ 0 };

    std::vector<Task*> dependents;
    std::mutex dependentsMutex;

    std::atomic<bool> enqueued{ false };    // evita dupla fila
};

// ============================================================
// BATCH WORK STEALING QUEUE
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

    // ========================================================
    // BATCH STEAL
    // ========================================================
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

    //int threadCount = std::thread::hardware_concurrency()
    //int threadCount = std::thread::hardware_concurrency() - 1
    //int threadCount = std::thread::hardware_concurrency() / 2
    JobSystem(int threadCount = std::thread::hardware_concurrency() / 2)
    {
        Init(threadCount);
    }

    ~JobSystem()
    {
        stop.store(true, std::memory_order_relaxed);
        cv.notify_all();

        for (auto& t : workers)
            t.join();
    }

    // ========================================================
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
    Task* CreateTask(void (*fn)(Task*, void*), void* data)
    {
        Task* t = new Task();
        t->function = fn;
        t->data = data;
        return t;
    }

    // ========================================================
    void AddDependency(Task* parent, Task* child)
    {
        JOB_LOG("[DEPENDENCY] " << parent << " -> " << child);

        child->dependencies.fetch_add(1, std::memory_order_relaxed);

        std::lock_guard<std::mutex> lock(parent->dependentsMutex);
        parent->dependents.push_back(child);
    }

    // ========================================================
    void TrySchedule(Task* t)
    {
        if (t->dependencies.load() != 0)
        { 
            JOB_LOG("[WAIT] Task " << t << " deps: " << t->dependencies.load() << "\n"); 
            return;
        }

        bool expected = false;
        if (!t->enqueued.compare_exchange_strong(expected, true))
            return;

        JOB_LOG("[SCHEDULE] Task " << t);

        Push(t);
    }

private:

    // ========================================================
    void Push(Task* t)
    {
        int idx = GetThreadIndex();
        queues[idx]->Push(t);

        cv.notify_one();
    }

    // ========================================================
    // WORKER LOOP (BATCH CORE)
    // ========================================================
    void Worker(int index)
    {
        threadIndex = index; 

        std::vector<Task*> batch;
        batch.reserve(reserveBatch/*64*/);

        while (!stop.load(std::memory_order_relaxed))
        {
            Task* task = nullptr;

            // 1. pega 1 local
            if (queues[index]->Pop(task))
            {
                ExecuteBatch(task, index);
                continue;
            }

            // 2. tenta steal em batch
            if (Steal(index, batch))
            {
                for (Task* t : batch)
                    ExecuteBatch(t, index);

                batch.clear();
                continue;
            }

            // sleep leve
            std::unique_lock<std::mutex> lock(cvMutex); 
            cv.wait(lock, [this]()          //cv.wait(lock);
            {
                return stop.load() || HasWork();
            });
        }
    }

    bool HasWork()
    {
        for (auto& q : queues)
        {
            if (!q->Empty())
                return true;
        }
        return false;
    }

    // ========================================================
    // BATCH STEAL
    // ========================================================
    bool Steal(int index, std::vector<Task*>& out)
    {
        for (int i = 0; i < queues.size(); i++)
        {
            if (i == index) continue;

            int stolen = queues[i]->StealBatch(out, maxStealBatch/*16*/);

            if (stolen > 0)
                return true;
        }

        return false;
    }

    // ========================================================
    // EXECUTION CORE
    // ========================================================
    void ExecuteBatch(Task* first, int index)
    {
        Task* current = first;

        //  batch chain execution (hot path optimization)
        for (int i = 0; i < chainBatchRun/*8*/ && current; i++)
        {
            JOB_LOG("[EXEC] Task " << current);

            current->function(current, current->data);

            Finish(current, index);

            // tenta pegar próxima local task sem voltar pro scheduler
            current = nullptr;
            queues[index]->Pop(current);
        }

        // se sobrar, re-enfileira implicitamente
        if (current)
            TrySchedule(current);
    }

    // ========================================================
    void Finish(Task* task, int index)
    {
        JOB_LOG("[FINISH] Task " << task);
        std::vector<Task*> deps;

        {
            std::lock_guard<std::mutex> lock(task->dependentsMutex);
            deps = task->dependents;
        }

        for (Task* d : deps)
        {
            if (d->dependencies.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                TrySchedule(d);
            }
        }

        delete task;
    }

    // ======================================================== 
    int GetThreadIndex()
    {
        if (threadIndex < 0)
            return 0;

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