#pragma once 
#include "../../ECS/Components/ComponentsHelper.h"
#include "../../Scene/SceneECS.h"
#include "../../ECS/System.h"
 
class ComputeSyncSystem : public System {
public:
    void Update(float deltaTime) override {
        try {
            // pega a entidade manager 
            Entity manager = GEngine->scene->computeManager;    //Entity manager = GEngine->scene->FindEntityByName("ComputeManager");
            if (manager == INVALID_ENTITY || !GEngine->scene->HasComponent<ComputeSyncComponent>(manager))
                return;

            auto& comp = GEngine->scene->GetComponent<ComputeSyncComponent>(manager);

            for (auto& s : comp.syncs) {
                if (!s.completed && s.sync) {
                    GLenum result = glClientWaitSync(s.sync, 0, 0);

                    if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED) {
                        s.completed = true;

                        if (s.onComplete) s.onComplete();

                        glDeleteSync(s.sync);
                        s.sync = 0;
                    }
                }
            }

            comp.syncs.erase(
                std::remove_if(comp.syncs.begin(), comp.syncs.end(),
                    [](auto& s) { return s.completed; }),
                comp.syncs.end()
            );

        }
        catch (const std::exception& e) {
            std::cerr << "[EXCEÇÃO - ComputeSyncSystem] " << e.what() << "\n";
        }
    }  
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