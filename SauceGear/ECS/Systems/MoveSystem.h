#pragma once

#include "../../Core/EngineContext.h"
#include "../Components/TransformComponent.h"
#include "../Components/Velocity.h"

class MoveSystem : public System {
public:
    void Update(float deltaTime) override {
        auto entities = GEngine->scene->GetEntitiesWith<TransformComponent, Velocity>();

        for (Entity e : entities) {
            auto& transform = GEngine->scene->GetComponent<TransformComponent>(e);
            auto& velocity = GEngine->scene->GetComponent<Velocity>(e);

            transform.position += velocity.velocity * deltaTime;
        }
    }
};
