#include "ThreadWorker.h"

ThreadPool gThreadPool;






/*



#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <glm/glm.hpp>
#include <memory>
#include <cmath>
#include <functional>
#include <iostream>

const unsigned int THREAD_COUNT = std::thread::hardware_concurrency();

// ============================================================
// THREAD POOL (Persistente / Profissional)
// ============================================================

class ThreadPool
{
public:
    ThreadPool() {
        stop = false;
        for (unsigned i = 0; i < THREAD_COUNT; ++i)
            workers.emplace_back([this] { Worker(); });
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        for (auto& t : workers) t.join();
    }

    template<class F>
    void Enqueue(F&& job)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            jobs.emplace(std::forward<F>(job));
        }
        cv.notify_one();
    }

private:
    void Worker()
    {
        while (true)
        {
            std::function<void()> job;

            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&] { return stop || !jobs.empty(); });

                if (stop && jobs.empty())
                    return;

                job = std::move(jobs.front());
                jobs.pop();
            }

            std::cout << "stt" << std::endl;
            job();
        }
    }

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> jobs;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop;
};

static ThreadPool gThreadPool;




*/