#pragma once

#include "Entity.h"

struct ScriptBehaviour {
    Entity entity;

    virtual void OnCreate() {}
    virtual void OnUpdate(float dt) {}  // = 0;
    virtual void OnDestroy() {}

    virtual ~ScriptBehaviour() = default;
};
