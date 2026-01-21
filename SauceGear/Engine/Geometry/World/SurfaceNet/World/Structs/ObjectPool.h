#pragma once

 #include <vector>
 #include <mutex>
 #include <chrono>
 #include <algorithm>
 #include <cstring>
 #include <glm/glm.hpp>

// Pool genérico parametrizado
template<typename T>
class ObjectPool {
public:
    using CreateFn = std::function<T* ()>;
    using DestroyFn = std::function<void(T*)>;
    using ResetFn = std::function<void(T*)>;

    struct Config {
        size_t maxObjects = 0; // 0 = ilimitado
        size_t minObjects = 0;
        std::chrono::seconds idleTimeout = std::chrono::seconds(30);
        bool threadSafe = true;
    };

    explicit ObjectPool(CreateFn create, DestroyFn destroy, ResetFn reset = nullptr, const Config& cfg = Config())
        : createFn(create), destroyFn(destroy), resetFn(reset), cfg(cfg) {
    }

    ~ObjectPool() { clearAll(); }

    // Obter objeto disponível ou criar novo
    T* acquire() {
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        if (cfg.threadSafe) lock.lock();

        // encontrar livre
        for (auto& entry : pool) {
            if (!entry.inUse) {
                entry.inUse = true;
                entry.lastUsed = std::chrono::steady_clock::now();
                if (resetFn) resetFn(entry.ptr);
                return entry.ptr;
            }
        }

        // criar novo se permitido
        if (cfg.maxObjects == 0 || pool.size() < cfg.maxObjects) {
            T* obj = createFn();
            pool.push_back({ obj, true, std::chrono::steady_clock::now() });
            return obj;
        }

        // fallback: reusar mais antigo livre (LRU)
        /*
        Entry* lru = nullptr;
        auto oldest = std::chrono::steady_clock::now();
        for (auto& e : pool)
            if (!e.inUse && e.lastUsed < oldest) { oldest = e.lastUsed; lru = &e; }

        if (lru) {
            if (resetFn) resetFn(lru->ptr);
            lru->inUse = true;
            lru->lastUsed = std::chrono::steady_clock::now();
            return lru->ptr;
        }
        */

        return nullptr; // saturado
    }

    //devolver objeto
    void release(T* obj) {
        if (!obj) return;
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        if (cfg.threadSafe) lock.lock();
        for (auto& entry : pool) {
            if (entry.ptr == obj) {
                entry.inUse = false;
                entry.lastUsed = std::chrono::steady_clock::now();
                return;
            }
        }
    }

    //faz analise e vai limpando os buffers livres
    void cleanupIdle() {
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        if (cfg.threadSafe) lock.lock();

        auto now = std::chrono::steady_clock::now();
        std::vector<Entry> newPool;
        for (auto& e : pool) {
            if (e.inUse) {
                newPool.push_back(e);
                continue;
            }
            auto idle = now - e.lastUsed;
            if (idle > cfg.idleTimeout && newPool.size() >= cfg.minObjects)
                destroyFn(e.ptr);
            else
                newPool.push_back(e);
        }
        pool.swap(newPool);
    }

    //limpa tudo e Destrói todos os objetos, usado no destrutor.
    void clearAll() {
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        if (cfg.threadSafe) lock.lock();
        for (auto& e : pool) destroyFn(e.ptr);
        pool.clear();
    }

private:
    struct Entry {
        T* ptr;
        bool inUse;
        std::chrono::steady_clock::time_point lastUsed;
    };

    CreateFn createFn;
    DestroyFn destroyFn;
    ResetFn resetFn;
    Config cfg;
    std::vector<Entry> pool;
    std::mutex mtx;

    static std::chrono::steady_clock::time_point now() { return std::chrono::steady_clock::now(); }
};

// Macros de conveniência para declarar/definir pools globais
#define DECLARE_GLOBAL_POOL(TYPE, NAME) extern ObjectPool<TYPE>* NAME;
#define DEFINE_GLOBAL_POOL(TYPE, NAME, CREATE_FN, DESTROY_FN, RESET_FN, CONFIG) \
    ObjectPool<TYPE>* NAME = new ObjectPool<TYPE>(CREATE_FN, DESTROY_FN, RESET_FN, CONFIG);
#define DELETE_GLOBAL_POOL(NAME) if(NAME){ delete NAME; NAME = nullptr; }


//Example
/*
ObjectPool<GLuint> bufferPool(
    [](){ GLuint id; glGenBuffers(1, &id); return new GLuint(id); },
    [](GLuint* p){ glDeleteBuffers(1, p); delete p; },
    [](GLuint* p){ glBindBuffer(GL_SHADER_STORAGE_BUFFER, *p); glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); }
);

GLuint* buf = bufferPool.acquire();
// ... usa o buffer
bufferPool.release(buf);

// ocasionalmente limpa inativos
bufferPool.cleanupIdle();

*/