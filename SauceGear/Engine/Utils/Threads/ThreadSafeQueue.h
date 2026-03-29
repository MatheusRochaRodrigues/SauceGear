#pragma once 
#include <queue>
#include <mutex>

template<typename T>
class ThreadSafeQueue
{
private:
    std::queue<T> queue;
    std::mutex mutex;

public:
    void push(const T& item)
    {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(item);
    }

    bool try_pop(T& out)
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (queue.empty())
            return false;

        out = queue.front();
        queue.pop();
        return true;
    }
};