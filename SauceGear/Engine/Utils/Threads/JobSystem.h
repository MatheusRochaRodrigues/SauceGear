#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <condition_variable>
#include "ObjectPool.h"
#include "JLOG.h"
  
constexpr int CHAIN_BATCH = 4; /*8*/
constexpr int MAX_STEAL_BATCH = 16;
constexpr int RESERVE_BATCH = 32; /*64*/

// ============================================================
// TASK
// ============================================================

//template<typename T>      void* to T
struct Task
{
    void (*function)(Task*, void*) = nullptr;
    void* data = nullptr;

    std::atomic<int> dependencies{ 0 };

    Task* nextDependent = nullptr; //   intrusive list (SEM vector)
    std::atomic<Task*> dependents{ nullptr };

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
     
    // BATCH STEAL 
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
 
// ~JOB SYSTEM  
class JobSystem
{
public: 
    JobSystem(int threadCount = std::thread::hardware_concurrency() / 1)    ////JobSystem(int threadCount = 3)
    {
        Init(threadCount);
         
        GetPool().PreallocateBlock(100000);     //GetPool().PreallocateBlockWithContructor(10000);
    }

    ~JobSystem()
    {
        stop.store(true, std::memory_order_relaxed);
        cv.notify_all();

        for (auto& t : workers) t.join();
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
     
    Task* CreateTask(void (*fn)(Task*, void*), void* data)
    {
        Task* t = GetPool().Acquire();   // pega 1 SLOT do bloco
        new (t) Task();                  // constrói o objeto

        t->function = fn;
        t->data = data;
        t->dependencies.store(0, std::memory_order_relaxed);
        t->dependents.store(nullptr, std::memory_order_relaxed);
        t->enqueued.store(false, std::memory_order_relaxed);
        
        //t->dependents.reserve(4); // evita realloc

        return t;
    }
     
    void AddDependency(Task* parent, Task* child)
    {
        JOB_LOG("[DEPENDENCY] " << parent << " -> " << child);

        child->dependencies.fetch_add(1, std::memory_order_relaxed);

        // intrusive lock-free push
        Task* head = parent->dependents.load();

        do {
            child->nextDependent = head;
        } while (!parent->dependents.compare_exchange_weak(head, child));

    }
     
    void TrySchedule(Task* t)
    {
        if (t->dependencies.load(std::memory_order_acquire) != 0) {
            JOB_LOG("[WAIT] Task " << t << " deps: " << t->dependencies.load() << "\n");
            return;
        }

        bool expected = false;
        if (!t->enqueued.compare_exchange_strong(expected, true))
            return;

        JOB_LOG("[SCHEDULE] Task " << t); 
        Push(t);
        JOB_LOG("[PUSH] Task " << t);
    }

private:
     
    void Push(Task* t)
    {
        int idx = GetThreadIndex();
        queues[idx]->Push(t);

        cv.notify_one();
    }
     
    // WORKER LOOP (BATCH CORE) 
    void Worker(int index)
    {
        threadIndex = index;

        std::vector<Task*> batch;
        batch.reserve(RESERVE_BATCH);

        while (!stop.load(std::memory_order_relaxed))
        {
            Task* task = nullptr;

            // get 1 local Task
            if (queues[index]->Pop(task))
            {
                ExecuteBatch(task, index);
                continue;
            }

            // or try to steal in batch
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

    bool HasWork() {
        for (auto& q : queues) {
            if (!q->Empty()) return true;
        }
        return false;
    }
     
    // BATCH STEAL  
    bool Steal(int index, std::vector<Task*>& out)
    {
        for (int i = 0; i < queues.size(); i++)
        {
            if (i == index) continue;

            if (queues[i]->StealBatch(out, MAX_STEAL_BATCH) > 0)
                return true;
        }
        return false;
    }
     
    // EXECUTION CORE 
    void ExecuteBatch(Task* first, int index)
    {
        Task* current = first;

        //  batch chain execution (hot path optimization)
        for (int i = 0; i < CHAIN_BATCH && current; i++)
        {
            JOB_LOG("[EXEC] Task " << current); 
            current->function(current, current->data); 
            ResolveDependencies(current);

            // tenta pegar próxima local task sem voltar pro scheduler
            current = nullptr;
            queues[index]->Pop(current);
        }

        // se sobrar, re-enfileira implicitamente
        if (current)
            TrySchedule(current);
    }     
     
    void ResolveDependencies(Task* task)    //Finish
    {
        Task* dep = task->dependents.load(std::memory_order_acquire);

        while (dep)
        {
            Task* next = dep->nextDependent;

            if (dep->dependencies.fetch_sub(1, std::memory_order_acq_rel) == 1)
                TrySchedule(dep);

            dep = next;
        }

        // AGORA SIM
        task->~Task();
        GetPool().Release(task);

        JOB_LOG("[FINISH] Task " << task);
    } 

    int GetThreadIndex()
    {
        if (threadIndex < 0)
            return 0;

        return threadIndex % queues.size();
    }

private: 
    //  TREADS  
    /*
    ObjectPool<Task>& GetPool()
    {
        static ObjectPool<Task> pool;
        return pool;
    }
    */

    static ObjectPool<Task, 128, 64>& GetPool()
    {
        static ObjectPool<Task, 128, 64> pool;          //ObjectPool<Task, 256, 128> pool;      ObjectPool<Task, 128, 64> g_taskPool;       
        //MENOS RAM MAS MAIS LENTO -> ObjectPool<Task, 64, 32> pool;
        return pool;
    } 

private:

    std::vector<std::thread> workers;
    std::vector<std::unique_ptr<WorkStealingQueue>> queues;

    std::atomic<bool> stop{ false };

    std::condition_variable cv;
    std::mutex cvMutex;

    static thread_local int threadIndex;
};














/*

    //  TREADS
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
*/






/*BlockPool<Task, 2048>& GetTaskPool()
{
    static BlockPool<Task, 2048> pool;
    return pool;
}*/

/*
ThreadArena<Task, 2048>& GetArenaPool()
{
    static ThreadArena<Task, 2048> arena(&GetTaskPool());
    return arena;
}
*/
// }



/* int GetThreadIndex() { return threadIndex < 0 ? 0 : threadIndex; } */


/*

    //int threadCount = std::thread::hardware_concurrency()
    //int threadCount = std::thread::hardware_concurrency() - 1
    //int threadCount = std::thread::hardware_concurrency() / 2

*/



/*
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
*/