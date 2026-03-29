#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <functional>
#include <iostream>

//const unsigned int THREAD_COUNT = std::thread::hardware_concurrency();
const unsigned int THREAD_COUNT = std::thread::hardware_concurrency()/2;
//const unsigned int THREAD_COUNT = 3;

class ThreadPool
{
public:
    ThreadPool() : stop(false), activeJobs(0)
    {
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
            jobs.emplace([this, job = std::forward<F>(job)]() {
                job();          // executa a tarefa
                activeJobs--;   // decrementa contador
                cvDone.notify_all();
                });
            activeJobs++; // incrementa contador ao adicionar tarefa
        }
        cv.notify_one();
    }

    void WaitAll()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cvDone.wait(lock, [this]() { return activeJobs == 0 && jobs.empty(); });
    }

private:
    void Worker()
    {
        while (true)
        {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]() { return stop || !jobs.empty(); });
                if (stop && jobs.empty()) return;
                job = std::move(jobs.front());
                jobs.pop();
            }

            try {
                job();
            }
            catch (const std::exception& e) {
                std::cerr << "ThreadPool job exception: " << e.what() << "\n";
            }
            catch (...) {
                std::cerr << "ThreadPool job unknown exception\n";
            }
        }
    }

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> jobs;
    std::mutex mtx;
    std::condition_variable cv;
    std::condition_variable cvDone;
    bool stop;
    std::atomic<int> activeJobs;
};

// ThreadPool global
extern ThreadPool gThreadPool; 


//static ThreadPool gThreadPool;



