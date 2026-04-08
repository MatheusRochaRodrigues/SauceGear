#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <functional>
#include <cassert>
#include <memory>
#include <condition_variable>

// ============================================================
// JOB
// ============================================================

struct JobCounter
{
    std::atomic<int> value;
    std::function<void()> onComplete;
};

//template<typename T>      void* to T
struct Job
{
    void (*function)(Job*, void*);
    void* data = nullptr;

    JobCounter* counter = nullptr;

    bool autoDelete = false;                    //std::function<void(void*)> deleter;
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
        pendingJobs.store(0);

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
        cv.notify_all(); // acorda todas threads

        for (auto& t : workers)
            t.join();
    }

    // ========================================================
    void Schedule(Job job)
    {
        pendingJobs.fetch_add(1);

        int index = GetThreadIndex();
        queues[index]->Push(job);

        cv.notify_one(); // acorda uma thread
    }

    // ========================================================
    JobCounter* Dispatch(int count, int batchSize,
        void (*func)(Job*, void*),
        void* userData,
        std::function<void()> onComplete = nullptr)
    {
        int jobCount = (count + batchSize - 1) / batchSize;

        JobCounter* counter = new JobCounter;
        counter->value.store(jobCount);
        counter->onComplete = onComplete;

        struct BatchData
        {
            int start;
            int end;
            void* user;
        };

        for (int i = 0; i < jobCount; i++)
        {
            BatchData* data = new BatchData{
                i * batchSize,
                std::min((i + 1) * batchSize, count),
                userData
            };

            Job job;
            job.function = func;
            job.data = data;
            job.counter = counter;
            job.autoDelete = true;

            Schedule(job);
        }

        return counter;
    }

    JobCounter* CreateCounter(int initial = 1, std::function<void()> onComplete = nullptr)
    {
        return new JobCounter{ initial, onComplete };
    }

private:

    void Worker(int index)
    {
        threadIndex = index;

        while (true)
        {
            Job job;

            // tenta pegar trabalho
            if (queues[index]->Pop(job))
            {
                Execute(job);
                continue;
            }

            // tenta roubar
            bool found = false;
            for (int i = 0; i < queues.size(); i++)
            {
                if (i == index) continue;

                if (queues[i]->Steal(job))
                {
                    Execute(job);
                    found = true;
                    break;
                }
            }

            if (found)
                continue;

            // dormir
            std::unique_lock<std::mutex> lock(cvMutex);
            cv.wait(lock, [this]()
                {
                    return stop || pendingJobs.load() > 0;
                });

            if (stop)
                return;
        }
    }

    void Execute(Job& job)
    {
        job.function(&job, job.data);

        // cuidado: isso só é seguro se TODOS os jobs usam new                        //delete job.data; 
        if (job.autoDelete)
            delete (char*)job.data; // ou (BatchData*)

        if (job.counter)
        {
            if (job.counter->value.fetch_sub(1) == 1)
            {
                if (job.counter->onComplete)
                    job.counter->onComplete();

                delete job.counter;
            }
        }

        pendingJobs.fetch_sub(1);
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
    std::atomic<int> pendingJobs = 0;

    std::condition_variable cv;
    std::mutex cvMutex;

    static thread_local int threadIndex;
};