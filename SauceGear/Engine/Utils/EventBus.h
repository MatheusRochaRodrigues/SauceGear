#pragma once 
#include <unordered_map>
#include <queue>
#include <mutex>
#include <vector>
#include <functional>
#include <typeindex>

class EventBus {
public:
    template<typename Event>
    using Handler = std::function<void(const Event&)>;

    template<typename Event>
    static void Subscribe(Handler<Event> handler) {
        std::lock_guard<std::mutex> lock(GetMutex<Event>());
        GetHandlers<Event>().push_back(handler);
    }

    template<typename Event>
    static void Emit(const Event& event) {
        std::lock_guard<std::mutex> lock(GetQueueMutex<Event>());
        GetQueue<Event>().push(event);
    }

    // chamado no MAIN THREAD
    template<typename Event>
    static void Dispatch() {
        std::lock_guard<std::mutex> qlock(GetQueueMutex<Event>());
        std::lock_guard<std::mutex> hlock(GetMutex<Event>());

        auto& queue = GetQueue<Event>();
        auto& handlers = GetHandlers<Event>();

        while (!queue.empty()) {
            const Event& e = queue.front();
            for (auto& h : handlers)
                h(e);
            queue.pop();
        }
    }

private:
    template<typename Event>
    static std::vector<Handler<Event>>& GetHandlers() {
        static std::vector<Handler<Event>> handlers;
        return handlers;
    }

    template<typename Event>
    static std::queue<Event>& GetQueue() {
        static std::queue<Event> queue;
        return queue;
    }

    template<typename Event>
    static std::mutex& GetMutex() {
        static std::mutex m;
        return m;
    }

    template<typename Event>
    static std::mutex& GetQueueMutex() {
        static std::mutex m;
        return m;
    }
};


/*
class EventBus {
public:
    template<typename Event>
    using Handler = std::function<void(const Event&)>;

    // Inscreve
    template<typename Event>
    static void Subscribe(Handler<Event> handler) {
        auto& handlers = GetHandlers<Event>();
        handlers.push_back(handler);
    }

    // Emite
    template<typename Event>
    static void Emit(const Event& event) {
        auto& handlers = GetHandlers<Event>();
        for (auto& h : handlers)
            h(event);
    }

private:
    template<typename Event>
    static std::vector<Handler<Event>>& GetHandlers() {
        static std::vector<Handler<Event>> handlers;
        return handlers;
    }
};
*/

/*
SceneECS
   |
   |  (emite evento)
   v
EventBus ---------------------
   |           |            |
 LightPass   Physics     Audio

*/
