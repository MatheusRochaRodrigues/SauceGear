#pragma once
#include <glad/glad.h>
#include <functional>
#include <vector>
#include <algorithm>

struct ComputeSyncEntry {
    GLsync sync = 0;
    bool completed = false;
    std::function<void()> onComplete;   //callback
};

struct ComputeSyncComponent {
    // Global/estática: todas as syncs enfileiradas aqui
    static inline std::vector<ComputeSyncEntry> syncs;

    // Enfileira trabalho já com fence
    static void Enqueue(GLsync s, std::function<void()> cb = nullptr) {
        if (!s) return;
        ComputeSyncEntry entry;
        entry.sync = s;
        entry.onComplete = std::move(cb);
        entry.completed = false;
        syncs.push_back(std::move(entry));
    }

    // Atalho estilo Unity: dispara compute e já cria fence
    static void Request(std::function<void()> gpuWork, std::function<void()> onComplete = nullptr) {
        // roda o trabalho GPU
        gpuWork();

        // força flush e coloca fence depois do trabalho
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        glFlush();

        Enqueue(sync, std::move(onComplete));
    }

    // Tem syncs pendentes?
    static bool HasPending() {
        for (auto& s : syncs) if (!s.completed) return true;
        return false;
    }

    // Reset geral (shutdown)
    static void Clear() {
        for (auto& s : syncs) {
            if (s.sync) {
                glDeleteSync(s.sync);
                s.sync = 0;
            }
        }
        syncs.clear();
    }
};
