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

//template<typename T>      void* to T
struct Task
{
    void (*function)(Task*, void*);
    void* data = nullptr;

    bool autoDelete = false;                        //std::function<void(void*)> deleter;

    // DEPENDÊNCIAS (fan-in)
    std::atomic<int> dependencies = 0;

    // CONTINUAÇÕES (fan-out)
    std::vector<Task*> dependents;
};

// ============================================================
// QUEUE
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
        pendingTasks = 0;

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
    // DEPENDENCY
    // ========================================================
    void AddDependency(Task* parent, Task* child)
    {
        child->dependencies.fetch_add(1);
        parent->dependents.push_back(child);
    }


    /*      to can run addDependency inside of one thread
    std::mutex depMutex;
    void AddDependency(Task* parent, Task* child)
    {
        child->dependencies.fetch_add(1);

        std::lock_guard<std::mutex> lock(depMutex);
        parent->dependents.push_back(child);
    }
    */

    // ========================================================
    // SCHEDULE
    // ========================================================
    void Schedule(Task* task)
    {
        if (task->dependencies.load() > 0)
            return;

        pendingTasks.fetch_add(1);

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

    void Execute(Task* task)
    {
        task->function(task, task->data);

        // LIBERA DEPENDENTES
        for (Task* dependent : task->dependents)
        {
            if (dependent->dependencies.fetch_sub(1) == 1)
            {
                Schedule(dependent);
            }
        }

        if (task->autoDelete)
            delete (char*)task->data;

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