#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>

struct GPUJob {
    std::function<void()> task;
};

class GPUJobQueue {
private:
    std::queue<GPUJob> jobs;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void Enqueue(GPUJob job) {
        std::lock_guard<std::mutex> lock(mtx);
        jobs.push(std::move(job));
        cv.notify_one();
    }

    bool TryPop(GPUJob& job) {
        std::lock_guard<std::mutex> lock(mtx);
        if (jobs.empty()) return false;
        job = std::move(jobs.front());
        jobs.pop();
        return true;
    }

    void WaitAndPop(GPUJob& job) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&] { return !jobs.empty(); });
        job = std::move(jobs.front());
        jobs.pop();
    }
};


//example
/*
void ProcessGPUJobs(GPUJobQueue& queue) {
    GPUJob job;
    while (queue.TryPop(job)) {
        job.task(); // aqui OpenGL é chamado na thread principal
    }
}
*/