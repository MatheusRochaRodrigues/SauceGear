#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>

class WorldComputeThread {
public:
    WorldComputeThread() : stop(false) {
        worker = std::thread(&WorldComputeThread::threadFunc, this);
    }

    ~WorldComputeThread() {
        { std::unique_lock<std::mutex> lk(mtx); stop = true; cv.notify_all(); }
        if (worker.joinable()) worker.join();
    }

    // Post a job that runs on the compute thread and returns a future
    template<typename Fn, typename... Args>
    auto submit(Fn&& f, Args&&... args) -> std::future<typename std::result_of<Fn(Args...)>::type> {
        using R = typename std::result_of<Fn(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<R()>>(
            std::bind(std::forward<Fn>(f), std::forward<Args>(args)...)
        );
        auto fut = task->get_future();
        {
            std::unique_lock<std::mutex> lk(mtx);
            jobs.emplace([task]() { (*task)(); });
        }
        cv.notify_one();
        return fut;
    }

private:
    std::thread worker;
    std::queue<std::function<void()>> jobs;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop;

    void threadFunc() {
        // Aqui: torna context GL atual nesta thread antes que ela comece a fazer GL.
        // **Você precisa criar/transferir contexto GL** e torná-lo current aqui.
        // Exemplo (GLFW): glfwMakeContextCurrent(windowForThisThread);
        // -----

        while (true) {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lk(mtx);
                cv.wait(lk, [&] { return stop || !jobs.empty(); });
                if (stop && jobs.empty()) break;
                job = std::move(jobs.front()); jobs.pop();
            }
            try { job(); }
            catch (...) { /* capture errors if desired */ }
        }

        // se necessário, destrói/contexto cleanup
    }
};
