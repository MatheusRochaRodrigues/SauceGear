#pragma once
#include "../System.h"
#include "../../Scene/SceneECS.h"
#include "../Components/ComputeSyncComponent.h" // o novo estático
#include <algorithm>
#include <iostream>

#ifndef DEBUG_COMPUTE_SYNC
#define DEBUG_COMPUTE_SYNC 0
#endif

class ComputeSyncSystem : public System {
public:
    void Update(float deltaTime) override {
        try {
            int callbacksExecuted = 0;

            for (auto& s : ComputeSyncComponent::syncs) {
                if (s.completed || !s.sync) continue;

                GLenum result = glClientWaitSync(s.sync, 0, 0);

                if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED) {
                    s.completed = true;

                    glDeleteSync(s.sync);
                    s.sync = 0;

                    if (s.onComplete) {
                        s.onComplete();
                        callbacksExecuted++;
                    }

                    #if DEBUG_COMPUTE_SYNC
                        std::cout << "[ComputeSyncSystem] Fence concluída, callback executado.\n";
                    #endif

                    if (callbacksExecuted >= maxCallbacksPerFrame)
                        break;
                }
            }

            // remove finalizados
            auto& syncs = ComputeSyncComponent::syncs;
            syncs.erase(
                std::remove_if(syncs.begin(), syncs.end(),
                    [](auto& s) { return s.completed; }),
                syncs.end()
            );
        }
        catch (const std::exception& e) {
            std::cerr << "[EXCEÇÃO - ComputeSyncSystem] " << e.what() << "\n";
        }
    }

private:
    int maxCallbacksPerFrame = 32; // throttle para não travar CPU
};




/*
void Update(float deltaTime) override {
    try {
        auto entities = GEngine->scene->GetEntitiesWith<ComputeSyncComponent>();
        for (Entity e : entities) {
            auto& syncComp = GEngine->scene->GetComponent<ComputeSyncComponent>(e);

            for (auto& s : syncComp.syncs) {
                if (!s.completed && s.sync) {
                    GLenum result = glClientWaitSync(s.sync, 0, 0); // polling

                    if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED) {
                        s.completed = true;

                        if (s.onComplete) s.onComplete();

                        glDeleteSync(s.sync);
                        s.sync = 0;
                    }
                }
            }

            // opcional: limpar todos os completados
            syncComp.syncs.erase(
                std::remove_if(syncComp.syncs.begin(), syncComp.syncs.end(),
                    [](const auto& s) { return s.completed; }),
                syncComp.syncs.end()
            );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[EXCEÇÃO - ComputeSyncSystem] " << e.what() << "\n";
    }
}
*/