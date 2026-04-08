#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <functional>
#include <memory>
#include <condition_variable>

// ============================================================
// TASK
// ============================================================

struct Task
{
    void (*function)(Task*, void*);
    void* data = nullptr;

    // dependências (fan-in)
    std::atomic<int> dependencies{ 0 };

    // dependentes (fan-out)
    std::vector<Task*> dependents;
    std::mutex dependentsMutex;
};

// ============================================================
// WORK STEALING QUEUE
// ============================================================

class WorkStealingQueue
{
public:
    void Push(Task* job)
    {
        std::lock_guard<std::mutex> lock(mtx);
        jobs.push_back(job);
    }

    bool Pop(Task*& job)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (jobs.empty()) return false;

        job = jobs.back();
        jobs.pop_back();
        return true;
    }

    bool Steal(Task*& job)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (jobs.empty()) return false;

        job = jobs.front();
        jobs.pop_front();
        return true;
    }

private:
    std::deque<Task*> jobs;
    std::mutex mtx;
};

// ============================================================
// TASK GRAPH SYSTEM
// ============================================================

class JobSystem
{
public:

    JobSystem() { Init(); }

    void Init(int threadCount = std::thread::hardware_concurrency())
    {
        stop = false;
        pendingTasks.store(0);

        queues.resize(threadCount);

        for (int i = 0; i < threadCount; i++)
        {
            queues[i] = std::make_unique<WorkStealingQueue>();

            workers.emplace_back([this, i]()
                {
                    Worker(i);
                });
        }
    }

    void Shutdown()
    {
        stop = true;
        cv.notify_all();

        for (auto& t : workers)
            t.join();
    }

    // ========================================================
    // CREATE TASK
    // ========================================================
    Task* CreateTask(void (*func)(Task*, void*), void* data = nullptr)
    {
        Task* t = new Task();
        t->function = func;
        t->data = data;
        return t;
    }

    // ========================================================
    // ADD DEPENDENCY
    // ========================================================
    void AddDependency(Task* parent, Task* child)
    {
        child->dependencies.fetch_add(1, std::memory_order_relaxed);

        std::lock_guard<std::mutex> lock(parent->dependentsMutex);
        parent->dependents.push_back(child);
    }

    // ========================================================
    // SCHEDULE
    // ========================================================
    void Schedule(Task* task)
    {
        pendingTasks.fetch_add(1);

        // Só agenda se já pode rodar
        if (task->dependencies.load(std::memory_order_acquire) > 0)
            return;

        //pendingTasks.fetch_add(1);

        int index = GetThreadIndex();
        queues[index]->Push(task);

        cv.notify_one();
    }

private:

    void Worker(int index)
    {
        threadIndex = index;

        while (true)
        {
            Task* task = nullptr;

            if (queues[index]->Pop(task))
            {
                Execute(task);
                continue;
            }

            bool found = false;

            for (int i = 0; i < queues.size(); i++)
            {
                if (i == index) continue;

                if (queues[i]->Steal(task))
                {
                    Execute(task);
                    found = true;
                    break;
                }
            }

            if (found)
                continue;

            std::unique_lock<std::mutex> lock(cvMutex);
            cv.wait(lock, [this]()
                {
                    return stop || pendingTasks.load() > 0;
                });

            if (stop)
                return;
        }
    }

    // ========================================================
    // EXECUTE
    // ========================================================
    void Execute(Task* task)
    {
        // roda a task
        task->function(task, task->data);

        // 🔥 pega dependentes com lock
        std::vector<Task*> localDependents;
        {
            std::lock_guard<std::mutex> lock(task->dependentsMutex);
            localDependents = task->dependents;
        }

        // 🔥 libera dependentes
        for (Task* dependent : localDependents)
        {
            if (dependent->dependencies.fetch_sub(1) == 1)
            {
                Schedule(dependent);
            }
        }

        pendingTasks.fetch_sub(1);

        delete task;
    }

    int GetThreadIndex()
    {
        if (threadIndex < 0)
            return 0;

        return threadIndex % queues.size();
    }

private:
    std::vector<std::thread> workers;
    std::vector<std::unique_ptr<WorkStealingQueue>> queues;

    std::atomic<bool> stop = false;
    std::atomic<int> pendingTasks = 0;

    std::condition_variable cv;
    std::mutex cvMutex;

    static thread_local int threadIndex;
};

//thread_local int JobSystem::threadIndex = -1;