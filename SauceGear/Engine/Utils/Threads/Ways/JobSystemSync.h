#pragma once

/*

#include <vector>
#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <functional>
#include <cassert>

// ============================================================
// JOB
// ============================================================

struct Job
{
    void (*function)(Job*, void*);
    void* data = nullptr;

    std::atomic<int>* counter = nullptr;
};

// ============================================================
// WORK STEALING QUEUE
// ============================================================

class WorkStealingQueue
{
public:
    void Push(Job job)
    {
        std::lock_guard<std::mutex> lock(mtx);
        jobs.push_back(job);
    }

    bool Pop(Job& job)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (jobs.empty()) return false;

        job = jobs.back();
        jobs.pop_back();
        return true;
    }

    bool Steal(Job& job)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (jobs.empty()) return false;

        job = jobs.front();
        jobs.pop_front();
        return true;
    }

private:
    std::deque<Job> jobs;
    std::mutex mtx;
};

// ============================================================
// JOB SYSTEM
// ============================================================

class JobSystem
{
public:

    JobSystem() { Init(); }

    void Init(int threadCount = std::thread::hardware_concurrency())
    {
        stop = false;
        queues.resize(threadCount);

        for (int i = 0; i < threadCount; i++)
        {
            workers.emplace_back([this, i]()
                {
                    Worker(i);
                });
        }
    }

    void Shutdown()
    {
        stop = true;

        for (auto& t : workers)
            t.join();
    }

    // ========================================================
    // EXECUTE SINGLE JOB
    // ========================================================
    void Schedule(Job job)
    {
        int index = GetThreadIndex();
        queues[index].Push(job);
    }

    // ========================================================
    // DISPATCH (PARALELISMO)
    // ========================================================
    std::atomic<int>* Dispatch(int count, int batchSize,
        void (*func)(Job*, void*),
        void* userData)
    {
        int jobCount = (count + batchSize - 1) / batchSize;

        auto* counter = new std::atomic<int>(jobCount);

        for (int i = 0; i < jobCount; i++)
        {
            struct BatchData
            {
                int start;
                int end;
                void* user;
            };

            BatchData* data = new BatchData{
                i * batchSize,
                std::min((i + 1) * batchSize, count),
                userData
            };

            Job job;
            job.function = func;
            job.data = data;
            job.counter = counter;

            Schedule(job);
        }

        return counter;
    }

    // ========================================================
    // WAIT
    // ========================================================
    void Wait(std::atomic<int>* counter)
    {
        while (counter->load() > 0)
        {
            std::this_thread::yield();
        }

        delete counter;
    }

    // ========================================================
    // HELPERS
    // ========================================================
    std::atomic<int>* CreateCounter(int initial = 1)
    {
        return new std::atomic<int>(initial);
    }

private:

    void Worker(int index)
    {
        threadIndex = index;

        while (!stop)
        {
            Job job;

            // 1. tenta pegar da própria fila
            if (queues[index].Pop(job))
            {
                Execute(job);
                continue;
            }

            // 2. tenta roubar de outras threads
            for (int i = 0; i < queues.size(); i++)
            {
                if (i == index) continue;

                if (queues[i].Steal(job))
                {
                    Execute(job);
                    break;
                }
            }
        }
    }

    void Execute(Job& job)
    {
        job.function(&job, job.data);

        if (job.counter)
            job.counter->fetch_sub(1);
    }

    int GetThreadIndex()
    {
        if (threadIndex < 0)
            return 0;

        return threadIndex % queues.size();
    }

private:
    std::vector<std::thread> workers;
    std::vector<WorkStealingQueue> queues;

    std::atomic<bool> stop = false;

    static thread_local int threadIndex;
};

thread_local int JobSystem::threadIndex = -1;

*/