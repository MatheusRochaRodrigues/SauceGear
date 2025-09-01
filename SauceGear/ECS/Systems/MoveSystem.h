#pragma once

#include "../../Core/EngineContext.h"
#include "../Components/Transform.h"
#include "../Components/Velocity.h"

class MoveSystem : public System {
public:
    void Update(float deltaTime) override {
        auto entities = GEngine->scene->GetEntitiesWith<Transform, Velocity>();

        for (Entity e : entities) {
            auto& transform = GEngine->scene->GetComponent<Transform>(e);
            auto& velocity = GEngine->scene->GetComponent<Velocity>(e);

            transform.position += velocity.velocity * deltaTime;
        }
    }
};
